// SPDX-License-Identifier: LGPL-3.0-or-later

#include "WASAPIAudioDevice_p.h"

#include <avrt.h>
#include <ks.h>
#include <ksmedia.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>

#include <QDebug>
#include <QLoggingCategory>
#include <QMetaObject>
#include <QMutexLocker>
#include <QProcess>
#include <QTimer>
#include <QWinEventNotifier>

#include <TalcsCore/AudioSource.h>

namespace talcs {

    Q_LOGGING_CATEGORY(lcWASAPIAudioDevice, "talcs.device.wasapi.device")

    static bool copyFormat(const WAVEFORMATEX *source, WAVEFORMATEXTENSIBLE &destination) {
        if (!source) {
            return false;
        }
        destination = {};
        std::memcpy(&destination, source, std::min(sizeof(destination), sizeof(WAVEFORMATEX) + size_t(source->cbSize)));
        const bool extensible = source->wFormatTag == WAVE_FORMAT_EXTENSIBLE;
        if (extensible && source->cbSize < sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX)) {
            return false;
        }
        const bool floating = source->wFormatTag == WAVE_FORMAT_IEEE_FLOAT || (extensible && destination.SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT);
        const bool pcm = source->wFormatTag == WAVE_FORMAT_PCM || (extensible && destination.SubFormat == KSDATAFORMAT_SUBTYPE_PCM);
        const auto bits = source->wBitsPerSample;
        const auto validBits = extensible ? destination.Samples.wValidBitsPerSample : bits;
        return source->nChannels > 0 && source->nSamplesPerSec > 0 && source->nBlockAlign == source->nChannels * (bits / 8) &&
            ((floating && bits == 32) || (pcm && (bits == 8 || bits == 16 || bits == 24 || bits == 32) && validBits > 0 && validBits <= bits));
    }

    WASAPIAudioWorker::WASAPIAudioWorker(WASAPIAudioDriver::ShareMode mode) : m_mode(mode) {
    }

    HRESULT WASAPIAudioWorker::initialize() {
        const auto result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        if (FAILED(result)) {
            return result;
        }
        m_ownsCom = true;
        m_watchdog = new QTimer(this);
        m_watchdog->setInterval(1000);
        connect(m_watchdog, &QTimer::timeout, this, [this] {
            DWORD state = 0;
            auto result = m_endpoint->GetState(&state);
            if (SUCCEEDED(result) && state != DEVICE_STATE_ACTIVE) {
                result = AUDCLNT_E_DEVICE_INVALIDATED;
            }
            UINT32 padding = 0;
            if (SUCCEEDED(result)) {
                result = m_client->GetCurrentPadding(&padding);
            }
            if (FAILED(result)) {
                stop();
                Q_EMIT streamFailed(m_generation, result);
            }
        });
        return CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_enumerator));
    }

    void WASAPIAudioWorker::finalize() {
        close();
        m_enumerator.Reset();
        if (m_ownsCom) {
            CoUninitialize();
            m_ownsCom = false;
        }
    }

    HRESULT WASAPIAudioWorker::activate(const QString &id) {
        if (!m_enumerator) {
            return CO_E_NOTINITIALIZED;
        }
        auto result = m_enumerator->GetDevice(reinterpret_cast<LPCWSTR>(id.utf16()), &m_endpoint);
        if (FAILED(result)) {
            qCWarning(lcWASAPIAudioDevice) << "WASAPI endpoint activation failed during GetDevice." << "endpoint:" << id << "error:" << wasapiErrorString(result);
            return result;
        }
        result = m_endpoint->Activate(__uuidof(IAudioClient3), CLSCTX_INPROC_SERVER, nullptr, reinterpret_cast<void **>(m_client.GetAddressOf()));
        if (FAILED(result)) {
            qCWarning(lcWASAPIAudioDevice) << "WASAPI IAudioClient3 activation failed." << "endpoint:" << id << "error:" << wasapiErrorString(result);
            return result;
        }
        WAVEFORMATEX *mixFormat = nullptr;
        result = m_client->GetMixFormat(&mixFormat);
        if (SUCCEEDED(result) && !copyFormat(mixFormat, m_format)) {
            result = AUDCLNT_E_UNSUPPORTED_FORMAT;
        }
        CoTaskMemFree(mixFormat);
        if (FAILED(result)) {
            qCWarning(lcWASAPIAudioDevice) << "WASAPI mix format retrieval failed." << "endpoint:" << id << "error:" << wasapiErrorString(result);
        }
        return result;
    }

    HRESULT WASAPIAudioWorker::selectFormat(double sampleRate) {
        if (m_mode == WASAPIAudioDriver::Shared) {
            return sampleRate == m_format.Format.nSamplesPerSec ? S_OK : AUDCLNT_E_UNSUPPORTED_FORMAT;
        }
        auto format = m_format;
        format.Format.nSamplesPerSec = static_cast<DWORD>(sampleRate);
        format.Format.nAvgBytesPerSec = format.Format.nSamplesPerSec * format.Format.nBlockAlign;
        if (m_client->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, &format.Format, nullptr) == S_OK) {
            m_format = format;
            return S_OK;
        }
        struct Encoding {
            WORD bits;
            WORD validBits;
            bool floating;
        };
        static constexpr Encoding encodings[] = {{32, 32, true}, {32, 32, false}, {32, 24, false}, {24, 24, false}, {16, 16, false}, {8, 8, false}};
        for (const auto encoding : encodings) {
            format.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
            format.Format.cbSize = sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX);
            format.Format.wBitsPerSample = encoding.bits;
            format.Format.nBlockAlign = format.Format.nChannels * (encoding.bits / 8);
            format.Format.nAvgBytesPerSec = format.Format.nSamplesPerSec * format.Format.nBlockAlign;
            format.Samples.wValidBitsPerSample = encoding.validBits;
            format.SubFormat = encoding.floating ? KSDATAFORMAT_SUBTYPE_IEEE_FLOAT : KSDATAFORMAT_SUBTYPE_PCM;
            if (m_client->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, &format.Format, nullptr) == S_OK) {
                m_format = format;
                return S_OK;
            }
            if (format.Format.nChannels <= 2 && encoding.bits == encoding.validBits) {
                format.Format.wFormatTag = encoding.floating ? WAVE_FORMAT_IEEE_FLOAT : WAVE_FORMAT_PCM;
                format.Format.cbSize = 0;
                if (m_client->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, &format.Format, nullptr) == S_OK) {
                    m_format = format;
                    return S_OK;
                }
            }
        }
        return AUDCLNT_E_UNSUPPORTED_FORMAT;
    }

    HRESULT WASAPIAudioWorker::probe(const QString &id, WASAPIDeviceCapabilities &capabilities) {
        close();
        auto result = activate(id);
        if (FAILED(result)) {
            return result;
        }
        capabilities = {};
        capabilities.channelCount = m_format.Format.nChannels;
        capabilities.preferredSampleRate = m_format.Format.nSamplesPerSec;
        if (m_mode == WASAPIAudioDriver::Shared) {
            UINT32 defaultPeriod = 0;
            UINT32 fundamentalPeriod = 0;
            UINT32 minPeriod = 0;
            UINT32 maxPeriod = 0;
            result = m_client->GetSharedModeEnginePeriod(&m_format.Format, &defaultPeriod, &fundamentalPeriod, &minPeriod, &maxPeriod);
            if (FAILED(result)) {
                return result;
            }
            if (!fundamentalPeriod || !minPeriod || minPeriod > maxPeriod) {
                return E_UNEXPECTED;
            }
            capabilities.sampleRates.append(capabilities.preferredSampleRate);
            capabilities.preferredBufferSize = defaultPeriod;
            const quint64 first = (quint64(minPeriod) + fundamentalPeriod - 1) / fundamentalPeriod * fundamentalPeriod;
            for (quint64 frames = first; frames <= maxPeriod; frames += fundamentalPeriod) {
                capabilities.bufferSizes.append(static_cast<qint64>(frames));
            }
            WAVEFORMATEX *currentFormat = nullptr;
            UINT32 currentPeriod = 0;
            if (SUCCEEDED(m_client->GetCurrentSharedModeEnginePeriod(&currentFormat, &currentPeriod)) &&
                currentFormat && currentFormat->nSamplesPerSec == capabilities.preferredSampleRate && capabilities.bufferSizes.contains(currentPeriod)) {
                capabilities.preferredBufferSize = currentPeriod;
            }
            CoTaskMemFree(currentFormat);
        } else {
            static const QList<double> sampleRates = {8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000, 64000, 88200, 96000, 128000, 176400, 192000, 256000, 352800, 384000};
            auto rates = sampleRates;
            if (!rates.contains(capabilities.preferredSampleRate)) {
                rates.append(capabilities.preferredSampleRate);
            }
            for (const auto rate : rates) {
                if (SUCCEEDED(selectFormat(rate))) {
                    capabilities.sampleRates.append(rate);
                }
            }
            if (capabilities.sampleRates.isEmpty()) {
                return AUDCLNT_E_UNSUPPORTED_FORMAT;
            }
            if (!capabilities.sampleRates.contains(capabilities.preferredSampleRate)) {
                capabilities.preferredSampleRate = capabilities.sampleRates.contains(48000) ? 48000 : capabilities.sampleRates.last();
            }
            REFERENCE_TIME defaultPeriod = 0;
            REFERENCE_TIME minPeriod = 0;
            result = m_client->GetDevicePeriod(&defaultPeriod, &minPeriod);
            if (FAILED(result)) {
                return result;
            }
            const qint64 minimum = std::max<qint64>(1, std::ceil(minPeriod * capabilities.preferredSampleRate / 10000000.0));
            capabilities.preferredBufferSize = std::max<qint64>(minimum, std::llround(defaultPeriod * capabilities.preferredSampleRate / 10000000.0));
            qint64 smallestBufferSize = minimum;
            for (const auto rate : capabilities.sampleRates) {
                const qint64 minFrames = std::max<qint64>(1, std::ceil(minPeriod * rate / 10000000.0));
                const qint64 defaultFrames = std::max<qint64>(minFrames, std::llround(defaultPeriod * rate / 10000000.0));
                capabilities.bufferSizes.append(minFrames);
                capabilities.bufferSizes.append(defaultFrames);
                smallestBufferSize = std::min(smallestBufferSize, minFrames);
            }
            for (const qint64 frames : {64, 128, 256, 512, 1024, 2048, 4096, 8192}) {
                if (frames >= smallestBufferSize) {
                    capabilities.bufferSizes.append(frames);
                }
            }
        }
        std::sort(capabilities.sampleRates.begin(), capabilities.sampleRates.end());
        std::sort(capabilities.bufferSizes.begin(), capabilities.bufferSizes.end());
        capabilities.bufferSizes.erase(std::unique(capabilities.bufferSizes.begin(), capabilities.bufferSizes.end()), capabilities.bufferSizes.end());
        return S_OK;
    }

    HRESULT WASAPIAudioWorker::open(const QString &id, qint64 blockSize, double sampleRate, int activeChannels, quint64 generation) {
        close();
        if (activeChannels <= 0) {
            return E_INVALIDARG;
        }
        m_generation = generation;
        auto result = activate(id);
        if (SUCCEEDED(result)) {
            result = selectFormat(sampleRate);
        }
        if (FAILED(result)) {
            return result;
        }
        if (m_mode == WASAPIAudioDriver::Shared) {
            UINT32 defaultPeriod = 0;
            UINT32 fundamentalPeriod = 0;
            UINT32 minPeriod = 0;
            UINT32 maxPeriod = 0;
            result = m_client->GetSharedModeEnginePeriod(&m_format.Format, &defaultPeriod, &fundamentalPeriod, &minPeriod, &maxPeriod);
            if (FAILED(result)) {
                return result;
            }
            if (!fundamentalPeriod || !minPeriod || minPeriod > maxPeriod) {
                return E_UNEXPECTED;
            }
            if (blockSize < minPeriod || blockSize > maxPeriod || blockSize % fundamentalPeriod != 0) {
                return AUDCLNT_E_INVALID_DEVICE_PERIOD;
            }
            const auto period = static_cast<UINT32>(blockSize);
            result = m_client->InitializeSharedAudioStream(AUDCLNT_STREAMFLAGS_EVENTCALLBACK, period, &m_format.Format, nullptr);
            qCDebug(lcWASAPIAudioDevice) << "WASAPI shared stream initialization returned." << "endpoint:" << id
                                       << "period:" << period << "result:" << wasapiErrorString(result);
        } else {
            REFERENCE_TIME defaultPeriod = 0;
            REFERENCE_TIME minPeriod = 0;
            result = m_client->GetDevicePeriod(&defaultPeriod, &minPeriod);
            if (FAILED(result)) {
                return result;
            }
            auto duration = std::llround(blockSize * 10000000.0 / sampleRate);
            if (duration < minPeriod) {
                return AUDCLNT_E_INVALID_DEVICE_PERIOD;
            }
            result = m_client->Initialize(AUDCLNT_SHAREMODE_EXCLUSIVE, AUDCLNT_STREAMFLAGS_EVENTCALLBACK, duration, duration, &m_format.Format, nullptr);
            if (result == AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED) {
                qCDebug(lcWASAPIAudioDevice) << "WASAPI exclusive buffer alignment is required." << "endpoint:" << id << "requested buffer size:" << blockSize;
                UINT32 alignedFrames = 0;
                result = m_client->GetBufferSize(&alignedFrames);
                if (SUCCEEDED(result)) {
                    m_client.Reset();
                    result = m_endpoint->Activate(__uuidof(IAudioClient3), CLSCTX_INPROC_SERVER, nullptr, reinterpret_cast<void **>(m_client.GetAddressOf()));
                }
                if (SUCCEEDED(result)) {
                    duration = std::llround(alignedFrames * 10000000.0 / sampleRate);
                    qCDebug(lcWASAPIAudioDevice) << "WASAPI exclusive stream initialization is retrying with an aligned buffer."
                                               << "endpoint:" << id << "aligned buffer size:" << alignedFrames;
                    result = m_client->Initialize(AUDCLNT_SHAREMODE_EXCLUSIVE, AUDCLNT_STREAMFLAGS_EVENTCALLBACK, duration, duration, &m_format.Format, nullptr);
                }
            }
        }
        if (FAILED(result)) {
            return result;
        }
        result = m_client->GetBufferSize(&m_endpointFrames);
        if (SUCCEEDED(result) && !m_endpointFrames) {
            return E_UNEXPECTED;
        }
        if (SUCCEEDED(result)) {
            result = m_client->GetService(IID_PPV_ARGS(&m_renderClient));
        }
        if (FAILED(result)) {
            return result;
        }
        m_audioEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
        if (!m_audioEvent) {
            return HRESULT_FROM_WIN32(GetLastError());
        }
        result = m_client->SetEventHandle(m_audioEvent);
        if (FAILED(result)) {
            return result;
        }
        m_floatFormat = m_format.Format.wFormatTag == WAVE_FORMAT_IEEE_FLOAT ||
            (m_format.Format.wFormatTag == WAVE_FORMAT_EXTENSIBLE && m_format.SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT);
        if (m_mode == WASAPIAudioDriver::Exclusive) {
            blockSize = m_endpointFrames;
        }
        m_buffer.resize(std::min(activeChannels, int(m_format.Format.nChannels)), blockSize);
        m_bufferPosition = blockSize;
        m_notifier = new QWinEventNotifier(m_audioEvent, this);
        m_notifier->setEnabled(false);
        connect(m_notifier, &QWinEventNotifier::activated, this, [this] { processAudio(); });
        return S_OK;
    }

    HRESULT WASAPIAudioWorker::start(AudioDeviceCallback *callback, quint64 generation) {
        if (!m_renderClient || !callback) {
            return E_UNEXPECTED;
        }
        stop();
        m_generation = generation;
        BYTE *data = nullptr;
        auto result = m_renderClient->GetBuffer(m_endpointFrames, &data);
        if (SUCCEEDED(result)) {
            result = m_renderClient->ReleaseBuffer(m_endpointFrames, AUDCLNT_BUFFERFLAGS_SILENT);
        }
        if (FAILED(result)) {
            return result;
        }
        m_callback = callback;
        m_bufferPosition = m_buffer.sampleCount();
        DWORD taskIndex = 0;
        m_mmcssTask = AvSetMmThreadCharacteristicsW(L"Pro Audio", &taskIndex);
        if (!m_mmcssTask) {
            const auto error = HRESULT_FROM_WIN32(GetLastError());
            qCWarning(lcWASAPIAudioDevice) << "WASAPI audio thread priority registration failed." << "error:" << wasapiErrorString(error);
        }
        m_notifier->setEnabled(true);
        result = m_client->Start();
        if (FAILED(result)) {
            stop();
            return result;
        }
        m_running = true;
        m_watchdog->start();
        return S_OK;
    }

    void WASAPIAudioWorker::stop() {
        if (m_watchdog) {
            m_watchdog->stop();
        }
        if (m_notifier) {
            m_notifier->setEnabled(false);
        }
        if (m_renderClient) {
            auto result = m_client->Stop();
            if (FAILED(result)) {
                qCWarning(lcWASAPIAudioDevice) << "WASAPI stream stop failed." << "error:" << wasapiErrorString(result);
            }
            result = m_client->Reset();
            if (FAILED(result)) {
                qCWarning(lcWASAPIAudioDevice) << "WASAPI stream reset failed." << "error:" << wasapiErrorString(result);
            }
        }
        m_running = false;
        m_callback = nullptr;
        if (m_audioEvent) {
            ResetEvent(m_audioEvent);
        }
        if (m_mmcssTask) {
            if (!AvRevertMmThreadCharacteristics(m_mmcssTask)) {
                const auto error = HRESULT_FROM_WIN32(GetLastError());
                qCWarning(lcWASAPIAudioDevice) << "WASAPI audio thread priority restoration failed." << "error:" << wasapiErrorString(error);
            }
            m_mmcssTask = nullptr;
        }
    }

    void WASAPIAudioWorker::close() {
        stop();
        delete m_notifier;
        m_notifier = nullptr;
        m_renderClient.Reset();
        m_client.Reset();
        m_endpoint.Reset();
        if (m_audioEvent) {
            CloseHandle(m_audioEvent);
            m_audioEvent = nullptr;
        }
        m_endpointFrames = 0;
        m_format = {};
    }

    qint64 WASAPIAudioWorker::bufferSize() const {
        return m_buffer.sampleCount();
    }

    void WASAPIAudioWorker::lock() {
        m_callbackMutex.lock();
    }

    void WASAPIAudioWorker::unlock() {
        m_callbackMutex.unlock();
    }

    HRESULT WASAPIAudioWorker::render() {
        UINT32 frames = m_endpointFrames;
        if (m_mode == WASAPIAudioDriver::Shared) {
            UINT32 padding = 0;
            const auto result = m_client->GetCurrentPadding(&padding);
            if (FAILED(result)) {
                return result;
            }
            if (padding > m_endpointFrames) {
                return E_UNEXPECTED;
            }
            frames -= padding;
        }
        if (!frames) {
            return S_OK;
        }
        BYTE *data = nullptr;
        auto result = m_renderClient->GetBuffer(frames, &data);
        if (FAILED(result)) {
            return result;
        }
        QMutexLocker locker(&m_callbackMutex);
        const auto bytesPerSample = m_format.Format.wBitsPerSample / 8;
        const int channels = m_format.Format.nChannels;
        const int bits = m_format.Format.wBitsPerSample;
        const int validBits = m_format.Format.wFormatTag == WAVE_FORMAT_EXTENSIBLE ? m_format.Samples.wValidBitsPerSample : bits;
        std::memset(data, bits == 8 ? 128 : 0, size_t(frames) * m_format.Format.nBlockAlign);
        for (UINT32 frame = 0; frame < frames; ++frame) {
            if (m_bufferPosition == m_buffer.sampleCount()) {
                m_buffer.clear();
                m_callback->workCallback(&m_buffer);
                m_bufferPosition = 0;
            }
            for (int channel = 0; channel < m_buffer.channelCount(); ++channel) {
                auto sample = m_buffer.constData(channel)[m_bufferPosition];
                if (!std::isfinite(sample)) {
                    sample = 0;
                }
                auto *destination = data + (size_t(frame) * channels + channel) * bytesPerSample;
                if (m_floatFormat) {
                    std::memcpy(destination, &sample, sizeof(sample));
                } else {
                    const qint64 scale = qint64(1) << (validBits - 1);
                    const auto value = std::clamp<qint64>(std::llround(std::clamp<double>(sample, -1, 1) * scale), -scale, scale - 1);
                    const auto shifted = value * (qint64(1) << (bits - validBits));
                    const auto encoded = bits == 8 ? quint32(shifted + 128) : quint32(shifted);
                    for (int byte = 0; byte < bytesPerSample; ++byte) {
                        destination[byte] = static_cast<BYTE>(encoded >> (byte * 8));
                    }
                }
            }
            ++m_bufferPosition;
        }
        return m_renderClient->ReleaseBuffer(frames, 0);
    }

    void WASAPIAudioWorker::processAudio() {
        if (!m_running) {
            return;
        }
        const auto result = render();
        if (FAILED(result)) {
            stop();
            Q_EMIT streamFailed(m_generation, result);
        }
    }

    WASAPIAudioDevice::WASAPIAudioDevice(const QString &name, const QString &endpointId, WASAPIAudioDriver *driver)
        : AudioDevice(*new WASAPIAudioDevicePrivate, driver) {
        Q_D(WASAPIAudioDevice);
        setName(name);
        setDriver(driver);
        qCInfo(lcWASAPIAudioDevice) << "WASAPI device initialization started." << "driver:" << driver->name() << "device:" << name << "endpoint:" << endpointId;
        d->endpointId = endpointId;
        d->worker = new WASAPIAudioWorker(driver->shareMode());
        d->worker->moveToThread(&d->workerThread);
        connect(&d->workerThread, &QThread::finished, d->worker, &QObject::deleteLater);
        connect(d->worker, &WASAPIAudioWorker::streamFailed, this, &WASAPIAudioDevice::handleStreamFailure, Qt::QueuedConnection);
        d->workerThread.start();
        HRESULT result = E_FAIL;
        QMetaObject::invokeMethod(d->worker, [&] { result = d->worker->initialize(); }, Qt::BlockingQueuedConnection);
        if (FAILED(result)) {
            setErrorString(wasapiErrorString(result));
            qCWarning(lcWASAPIAudioDevice) << "WASAPI audio worker initialization failed." << "device:" << name << "error:" << errorString();
            return;
        }
        d->workerInitialized = true;
        updateCapabilities();
    }

    WASAPIAudioDevice::~WASAPIAudioDevice() {
        Q_D(WASAPIAudioDevice);
        WASAPIAudioDevice::close();
        QMetaObject::invokeMethod(d->worker, [&] { d->worker->finalize(); }, Qt::BlockingQueuedConnection);
        d->workerThread.quit();
        d->workerThread.wait();
    }

    bool WASAPIAudioDevice::updateCapabilities() {
        Q_D(WASAPIAudioDevice);
        WASAPIDeviceCapabilities capabilities;
        HRESULT result = E_FAIL;
        QMetaObject::invokeMethod(d->worker, [&] {
            result = d->worker->probe(d->endpointId, capabilities);
            d->worker->close();
        }, Qt::BlockingQueuedConnection);
        if (FAILED(result)) {
            setErrorString(wasapiErrorString(result));
            qCWarning(lcWASAPIAudioDevice) << "WASAPI device capability probe failed." << "driver:" << driver()->name()
                                         << "device:" << name() << "endpoint:" << d->endpointId << "error:" << errorString();
            setIsInitialized(false);
            return false;
        }
        setChannelCount(capabilities.channelCount);
        setAvailableSampleRates(capabilities.sampleRates);
        setPreferredSampleRate(capabilities.preferredSampleRate);
        setAvailableBufferSizes(capabilities.bufferSizes);
        setPreferredBufferSize(capabilities.preferredBufferSize);
        setIsInitialized(true);
        qCDebug(lcWASAPIAudioDevice) << "WASAPI device capabilities resolved." << "driver:" << driver()->name() << "device:" << name()
                                   << "channels:" << capabilities.channelCount << "sample rates:" << capabilities.sampleRates
                                   << "buffer sizes:" << capabilities.bufferSizes << "preferred sample rate:" << capabilities.preferredSampleRate
                                   << "preferred buffer size:" << capabilities.preferredBufferSize;
        return true;
    }

    bool WASAPIAudioDevice::open(qint64 bufferSize, double sampleRate) {
        Q_D(WASAPIAudioDevice);
        qCInfo(lcWASAPIAudioDevice) << "WASAPI device open started." << "device:" << name() << "buffer size:" << bufferSize << "sample rate:" << sampleRate;
        close();
        if (!driver() || !driver()->isInitialized() || !d->workerInitialized) {
            setErrorString(tr("The audio driver is not initialized."));
            qCWarning(lcWASAPIAudioDevice) << "WASAPI device open failed." << "device:" << name() << "error:" << errorString();
            return false;
        }
        if (bufferSize <= 0 || bufferSize > std::numeric_limits<int>::max() || !std::isfinite(sampleRate) || sampleRate < 1 ||
            sampleRate > std::numeric_limits<DWORD>::max() || std::floor(sampleRate) != sampleRate) {
            setErrorString(tr("The requested audio buffer size or sample rate is invalid."));
            qCWarning(lcWASAPIAudioDevice) << "WASAPI device open failed." << "device:" << name()
                                         << "buffer size:" << bufferSize << "sample rate:" << sampleRate << "error:" << errorString();
            return false;
        }
        if (!updateCapabilities()) {
            return false;
        }
        HRESULT result = E_FAIL;
        qint64 actualBufferSize = 0;
        const int channels = activeChannelCount();
        QMetaObject::invokeMethod(d->worker, [&] {
            result = d->worker->open(d->endpointId, bufferSize, sampleRate, channels, d->generation);
            actualBufferSize = d->worker->bufferSize();
            if (FAILED(result)) {
                d->worker->close();
            }
        }, Qt::BlockingQueuedConnection);
        if (FAILED(result)) {
            setErrorString(wasapiErrorString(result));
            qCWarning(lcWASAPIAudioDevice) << "WASAPI stream open failed." << "driver:" << driver()->name() << "device:" << name()
                                         << "buffer size:" << bufferSize << "sample rate:" << sampleRate << "channels:" << channels << "error:" << errorString();
            return false;
        }
        setErrorString({});
        auto bufferSizes = availableBufferSizes();
        if (!bufferSizes.contains(actualBufferSize)) {
            bufferSizes.append(actualBufferSize);
            std::sort(bufferSizes.begin(), bufferSizes.end());
            setAvailableBufferSizes(bufferSizes);
        }
        auto sampleRates = availableSampleRates();
        if (!sampleRates.contains(sampleRate)) {
            sampleRates.append(sampleRate);
            std::sort(sampleRates.begin(), sampleRates.end());
            setAvailableSampleRates(sampleRates);
        }
        qCInfo(lcWASAPIAudioDevice) << "WASAPI device open completed." << "driver:" << driver()->name() << "device:" << name()
                                  << "buffer size:" << actualBufferSize << "sample rate:" << sampleRate << "channels:" << channels;
        return AudioStreamBase::open(actualBufferSize, sampleRate);
    }

    void WASAPIAudioDevice::close() {
        Q_D(WASAPIAudioDevice);
        const bool wasOpen = isOpen();
        stop();
        QMetaObject::invokeMethod(d->worker, [&] { d->worker->close(); }, Qt::BlockingQueuedConnection);
        AudioDevice::close();
        if (wasOpen) {
            qCInfo(lcWASAPIAudioDevice) << "WASAPI device closed." << "device:" << name();
        }
    }

    bool WASAPIAudioDevice::start(AudioDeviceCallback *callback) {
        Q_D(WASAPIAudioDevice);
        qCInfo(lcWASAPIAudioDevice) << "WASAPI playback start requested." << "device:" << name();
        if (!isOpen() || !callback) {
            setErrorString(tr("The audio device must be open before playback starts."));
            qCWarning(lcWASAPIAudioDevice) << "WASAPI playback start failed." << "device:" << name() << "callback present:" << bool(callback) << "error:" << errorString();
            return false;
        }
        stop();
        if (!callback->deviceWillStartCallback(this)) {
            qCWarning(lcWASAPIAudioDevice) << "WASAPI playback preparation callback failed." << "device:" << name();
            return false;
        }
        d->callback = callback;
        AudioDevice::start(callback);
        HRESULT result = E_FAIL;
        QMetaObject::invokeMethod(d->worker, [&] { result = d->worker->start(callback, d->generation); }, Qt::BlockingQueuedConnection);
        if (FAILED(result)) {
            setErrorString(wasapiErrorString(result));
            qCWarning(lcWASAPIAudioDevice) << "WASAPI playback start failed." << "driver:" << driver()->name() << "device:" << name() << "error:" << errorString();
            stop();
            return false;
        }
        qCInfo(lcWASAPIAudioDevice) << "WASAPI playback started." << "device:" << name();
        return true;
    }

    void WASAPIAudioDevice::stop() {
        Q_D(WASAPIAudioDevice);
        const bool wasStarted = isStarted();
        ++d->generation;
        QMetaObject::invokeMethod(d->worker, [&] { d->worker->stop(); }, Qt::BlockingQueuedConnection);
        auto *callback = d->callback;
        d->callback = nullptr;
        AudioDevice::stop();
        if (callback) {
            callback->deviceStoppedCallback();
        }
        if (wasStarted) {
            qCInfo(lcWASAPIAudioDevice) << "WASAPI playback stopped." << "device:" << name();
        }
    }

    void WASAPIAudioDevice::lock() {
        Q_D(WASAPIAudioDevice);
        d->worker->lock();
    }

    void WASAPIAudioDevice::unlock() {
        Q_D(WASAPIAudioDevice);
        d->worker->unlock();
    }

    bool WASAPIAudioDevice::openControlPanel() {
        QProcess process;
        process.setProgram(QStringLiteral("control.exe"));
        process.setArguments({QStringLiteral("mmsys.cpl")});
        if (!process.startDetached()) {
            qCWarning(lcWASAPIAudioDevice) << "WASAPI control panel launch failed." << "device:" << name() << "error:" << process.errorString();
            return false;
        }
        return true;
    }

    void WASAPIAudioDevice::handleEndpointChange(const QList<WASAPIEndpoint> &endpoints, const QSet<QString> &changedFormats) {
        Q_D(WASAPIAudioDevice);
        if (!d->workerInitialized) {
            return;
        }
        const auto endpoint = std::find_if(endpoints.cbegin(), endpoints.cend(), [d](const WASAPIEndpoint &endpoint) { return endpoint.id == d->endpointId; });
        const bool available = endpoint != endpoints.cend();
        if (available) {
            setName(endpoint->name);
        }
        const bool wasClosed = isOpen() && (!available || changedFormats.contains(d->endpointId));
        if (wasClosed) {
            qCWarning(lcWASAPIAudioDevice) << "WASAPI device is closing after an endpoint change." << "device:" << name()
                                         << "available:" << available << "format changed:" << changedFormats.contains(d->endpointId);
            close();
        }
        if (!available) {
            setIsInitialized(false);
        } else if (!isOpen()) {
            updateCapabilities();
        }
        if (wasClosed) {
            Q_EMIT closed();
        }
    }

    void WASAPIAudioDevice::handleStreamFailure(quint64 generation, HRESULT result) {
        Q_D(WASAPIAudioDevice);
        if (generation != d->generation || !isOpen()) {
            return;
        }
        setErrorString(wasapiErrorString(result));
        qCWarning(lcWASAPIAudioDevice) << "WASAPI audio stream failed." << "driver:" << (driver() ? driver()->name() : QString())
                                     << "device:" << name() << "endpoint:" << d->endpointId << "error:" << errorString();
        if (driver() && (result == AUDCLNT_E_DEVICE_INVALIDATED || result == AUDCLNT_E_RESOURCES_INVALIDATED)) {
            auto *wasapiDriver = static_cast<WASAPIAudioDriver *>(driver());
            wasapiDriver->requestRefresh();
        }
        close();
        Q_EMIT closed();
    }

}
