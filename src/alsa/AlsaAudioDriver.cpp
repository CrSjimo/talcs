// SPDX-FileCopyrightText: 2026 CrSjimo
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "AlsaAudioDriver.h"
#include "AlsaAudioDriver_p.h"

#include <algorithm>
#include <cerrno>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <limits>

#include <TalcsDevice/private/NativeAudioDevice_p_p.h>

namespace talcs {

    static int setFormat(snd_pcm_t *pcm, snd_pcm_hw_params_t *parameters, snd_pcm_format_t &format) {
        static const snd_pcm_format_t formats[] = {
            SND_PCM_FORMAT_FLOAT, SND_PCM_FORMAT_S32, SND_PCM_FORMAT_S24,
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
            SND_PCM_FORMAT_S24_3LE,
#else
            SND_PCM_FORMAT_S24_3BE,
#endif
            SND_PCM_FORMAT_S16,
        };
        for (const auto candidate : formats) {
            if (snd_pcm_hw_params_test_format(pcm, parameters, candidate) == 0) {
                format = candidate;
                return snd_pcm_hw_params_set_format(pcm, parameters, candidate);
            }
        }
        return -EINVAL;
    }

    AlsaAudioDriver::AlsaAudioDriver(QObject *parent) : AudioDriver(*new AlsaAudioDriverPrivate, parent) {
        Q_D(AlsaAudioDriver);
        setName(QStringLiteral("ALSA"));
        d->timer = new QTimer(this);
        d->timer->setInterval(1000);
        connect(d->timer, &QTimer::timeout, this, [d] { d->refresh(); });
    }

    AlsaAudioDriver::~AlsaAudioDriver() {
        AlsaAudioDriver::finalize();
    }

    bool AlsaAudioDriver::initialize() {
        Q_D(AlsaAudioDriver);
        if (isInitialized()) {
            return true;
        }
        if (!d->refresh()) {
            return false;
        }
        d->timer->start();
        setErrorString({});
        return AudioDriver::initialize();
    }

    bool AlsaAudioDriverPrivate::refresh() {
        Q_Q(AlsaAudioDriver);
        void **hints = nullptr;
        const auto result = snd_device_name_hint(-1, "pcm", &hints);
        if (result < 0) {
            q->setErrorString(QString::fromLocal8Bit(snd_strerror(result)));
            return false;
        }
        const auto previous = q->devices();
        endpoints.clear();
        for (auto **hint = hints; hint && *hint; ++hint) {
            char *id = snd_device_name_get_hint(*hint, "NAME");
            char *description = snd_device_name_get_hint(*hint, "DESC");
            char *direction = snd_device_name_get_hint(*hint, "IOID");
            if (id && strcmp(id, "null") && (!direction || strcmp(direction, "Input"))) {
                AlsaEndpoint endpoint;
                endpoint.id = QString::fromLocal8Bit(id);
                const auto text = QString::fromLocal8Bit(description).section(u'\n', 0, 0);
                endpoint.name = text.isEmpty() ? endpoint.id : QStringLiteral("%1 [%2]").arg(text, endpoint.id);
                const auto duplicate = std::any_of(endpoints.cbegin(), endpoints.cend(), [&endpoint](const AlsaEndpoint &other) { return other.id == endpoint.id; });
                if (!duplicate) {
                    endpoints.append(endpoint);
                }
            }
            free(id);
            free(description);
            free(direction);
        }
        if (hints) {
            snd_device_name_free_hint(hints);
        }
        for (auto *child : q->children()) {
            auto *device = qobject_cast<AlsaAudioDevice *>(child);
            if (device) {
                AlsaEndpoint current;
                for (const auto &endpoint : endpoints) {
                    if (endpoint.name == device->name()) {
                        current = endpoint;
                        break;
                    }
                }
                device->updateEndpoint(current);
            }
        }
        if (previous != q->devices()) {
            Q_EMIT q->deviceChanged();
        }
        return true;
    }

    void AlsaAudioDriver::finalize() {
        Q_D(AlsaAudioDriver);
        d->timer->stop();
        AudioDriver::finalize();
        for (auto *child : children()) {
            if (auto *device = qobject_cast<AlsaAudioDevice *>(child)) {
                device->close();
            }
        }
        d->endpoints.clear();
    }

    QStringList AlsaAudioDriver::devices() const {
        Q_D(const AlsaAudioDriver);
        QStringList names;
        for (const auto &endpoint : d->endpoints) {
            names.append(endpoint.name);
        }
        return names;
    }

    QString AlsaAudioDriver::defaultDevice() const {
        Q_D(const AlsaAudioDriver);
        for (const auto &endpoint : d->endpoints) {
            if (endpoint.id == QStringLiteral("default")) {
                return endpoint.name;
            }
        }
        return devices().value(0);
    }

    AudioDevice *AlsaAudioDriver::createDevice(const QString &name) {
        Q_D(AlsaAudioDriver);
        for (const auto &endpoint : d->endpoints) {
            if (endpoint.name == name) {
                return new AlsaAudioDevice(endpoint, this);
            }
        }
        setErrorString(tr("The audio device is no longer available."));
        return nullptr;
    }

    AlsaAudioDevice::AlsaAudioDevice(const AlsaEndpoint &endpoint, AlsaAudioDriver *driver)
        : NativeAudioDevice(*new NativeAudioDevicePrivate, driver), m_endpoint(endpoint) {
        setDriver(driver);
        setName(endpoint.name);
        setIsInitialized(probe());
    }

    AlsaAudioDevice::~AlsaAudioDevice() {
        AlsaAudioDevice::close();
    }

    bool AlsaAudioDevice::probe() {
        snd_pcm_t *pcm = nullptr;
        auto result = snd_pcm_open(&pcm, m_endpoint.id.toLocal8Bit().constData(), SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
        if (result < 0) {
            setErrorString(QString::fromLocal8Bit(snd_strerror(result)));
            return false;
        }
        snd_pcm_hw_params_t *parameters;
        snd_pcm_hw_params_alloca(&parameters);
        result = snd_pcm_hw_params_any(pcm, parameters);
        if (result >= 0) {
            result = snd_pcm_hw_params_set_access(pcm, parameters, SND_PCM_ACCESS_RW_INTERLEAVED);
        }
        unsigned int channels = 0;
        if (result >= 0) {
            result = snd_pcm_hw_params_get_channels_max(parameters, &channels);
        }
        // Virtual PCM configurations negotiate a stereo layout instead of a maximum channel count.
        if (snd_pcm_type(pcm) != SND_PCM_TYPE_HW) {
            channels = 2;
        }
        if (channels > unsigned(std::numeric_limits<int>::max())) {
            result = -EINVAL;
        }
        if (result >= 0) {
            result = snd_pcm_hw_params_set_channels_near(pcm, parameters, &channels);
        }
        if (result < 0) {
            snd_pcm_close(pcm);
            setErrorString(QString::fromLocal8Bit(snd_strerror(result)));
            return false;
        }
        snd_pcm_hw_params_t *candidate;
        snd_pcm_hw_params_alloca(&candidate);
        QList<double> rates;
        for (const auto rate : {8000U, 11025U, 16000U, 22050U, 32000U, 44100U, 48000U, 88200U, 96000U, 176400U, 192000U}) {
            snd_pcm_hw_params_copy(candidate, parameters);
            if (result >= 0 && snd_pcm_hw_params_set_rate(pcm, candidate, rate, 0) == 0 && setFormat(pcm, candidate, m_format) == 0) {
                rates.append(rate);
            }
        }
        unsigned int rate = rates.contains(48000) || rates.isEmpty() ? 48000 : unsigned(rates.first());
        if (result >= 0) {
            result = snd_pcm_hw_params_set_rate_near(pcm, parameters, &rate, nullptr);
        }
        if (result >= 0 && !rates.contains(rate)) {
            rates.append(rate);
            std::sort(rates.begin(), rates.end());
        }
        if (result < 0) {
            snd_pcm_close(pcm);
            setErrorString(QString::fromLocal8Bit(snd_strerror(result)));
            return false;
        }
        QList<qint64> sizes;
        for (const auto frames : {64, 128, 256, 512, 1024, 2048, 4096, 8192}) {
            snd_pcm_hw_params_copy(candidate, parameters);
            snd_pcm_uframes_t ringFrames = snd_pcm_uframes_t(frames * 2);
            if (result >= 0 && snd_pcm_hw_params_set_period_size(pcm, candidate, frames, 0) == 0 && snd_pcm_hw_params_set_buffer_size_near(pcm, candidate, &ringFrames) == 0 && ringFrames >= snd_pcm_uframes_t(frames * 2) && setFormat(pcm, candidate, m_format) == 0) {
                sizes.append(frames);
            }
        }
        snd_pcm_uframes_t frames = sizes.contains(1024) || sizes.isEmpty() ? 1024 : snd_pcm_uframes_t(sizes.first());
        if (result >= 0) {
            result = snd_pcm_hw_params_set_period_size_near(pcm, parameters, &frames, nullptr);
        }
        snd_pcm_uframes_t ringFrames = frames * 2;
        if (result >= 0) {
            result = snd_pcm_hw_params_set_buffer_size_near(pcm, parameters, &ringFrames);
        }
        if (result >= 0) {
            result = ringFrames >= frames * 2 ? setFormat(pcm, parameters, m_format) : -EINVAL;
        }
        snd_pcm_close(pcm);
        if (result < 0 || !channels || !rate || !frames || frames > 1048576) {
            setErrorString(result < 0 ? QString::fromLocal8Bit(snd_strerror(result)) : tr("The ALSA device has no supported audio format."));
            return false;
        }
        if (!sizes.contains(qint64(frames))) {
            sizes.append(qint64(frames));
            std::sort(sizes.begin(), sizes.end());
        }
        setChannelCount(int(channels));
        setAvailableSampleRates(rates);
        setPreferredSampleRate(rate);
        setAvailableBufferSizes(sizes);
        setPreferredBufferSize(qint64(frames));
        return true;
    }

    bool AlsaAudioDevice::open(qint64 bufferSize, double sampleRate) {
        close();
        if (!isInitialized() || !configure(bufferSize, sampleRate)) {
            return false;
        }
        auto result = snd_pcm_open(&m_pcm, m_endpoint.id.toLocal8Bit().constData(), SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
        snd_pcm_hw_params_t *parameters;
        snd_pcm_hw_params_alloca(&parameters);
        if (result >= 0) {
            result = snd_pcm_hw_params_any(m_pcm, parameters);
        }
        if (result >= 0) {
            result = snd_pcm_hw_params_set_access(m_pcm, parameters, SND_PCM_ACCESS_RW_INTERLEAVED);
        }
        if (result >= 0) {
            result = snd_pcm_hw_params_set_channels(m_pcm, parameters, unsigned(activeChannelCount()));
        }
        if (result >= 0) {
            result = snd_pcm_hw_params_set_rate(m_pcm, parameters, unsigned(sampleRate), 0);
        }
        // TALCS blocks correspond to ALSA periods. The hardware ring buffer contains multiple periods.
        if (result >= 0) {
            result = snd_pcm_hw_params_set_period_size(m_pcm, parameters, snd_pcm_uframes_t(bufferSize), 0);
        }
        snd_pcm_uframes_t ringFrames = snd_pcm_uframes_t(bufferSize * 2);
        if (result >= 0) {
            result = snd_pcm_hw_params_set_buffer_size_near(m_pcm, parameters, &ringFrames);
        }
        if (result >= 0 && ringFrames < snd_pcm_uframes_t(bufferSize * 2)) {
            result = -EINVAL;
        }
        if (result >= 0) {
            result = setFormat(m_pcm, parameters, m_format);
        }
        if (result >= 0) {
            result = snd_pcm_hw_params(m_pcm, parameters);
        }
        snd_pcm_sw_params_t *software;
        snd_pcm_sw_params_alloca(&software);
        if (result >= 0) {
            result = snd_pcm_sw_params_current(m_pcm, software);
        }
        if (result >= 0) {
            result = snd_pcm_sw_params_set_start_threshold(m_pcm, software, ringFrames - snd_pcm_uframes_t(bufferSize));
        }
        if (result >= 0) {
            result = snd_pcm_sw_params_set_avail_min(m_pcm, software, snd_pcm_uframes_t(bufferSize));
        }
        if (result >= 0) {
            result = snd_pcm_sw_params(m_pcm, software);
        }
        if (result < 0) {
            setErrorString(QString::fromLocal8Bit(snd_strerror(result)));
            close();
            return false;
        }
        m_frameBytes = snd_pcm_format_physical_width(m_format) / 8 * activeChannelCount();
        m_output.resize(size_t(bufferSize) * activeChannelCount());
        m_nativeOutput.resize(size_t(bufferSize) * m_frameBytes);
        return true;
    }

    bool AlsaAudioDevice::activateStream() {
        const auto result = snd_pcm_prepare(m_pcm);
        if (result < 0) {
            setErrorString(QString::fromLocal8Bit(snd_strerror(result)));
            return false;
        }
        m_stopping = false;
        m_worker = QThread::create([this] { process(); });
        m_worker->start(QThread::TimeCriticalPriority);
        return true;
    }

    void AlsaAudioDevice::deactivateStream() {
        m_stopping = true;
        if (m_worker) {
            m_worker->wait();
            delete m_worker;
            m_worker = nullptr;
        }
        if (m_pcm) {
            snd_pcm_drop(m_pcm);
        }
    }

    void AlsaAudioDevice::close() {
        stop();
        if (m_pcm) {
            snd_pcm_close(m_pcm);
            m_pcm = nullptr;
        }
        m_output.clear();
        m_nativeOutput.clear();
        AudioDevice::close();
    }

    bool AlsaAudioDevice::recover(int error) {
        if (error == -EINTR) {
            return true;
        }
        if (error == -EPIPE) {
            error = snd_pcm_prepare(m_pcm);
        } else if (error == -ESTRPIPE) {
            do {
                error = snd_pcm_resume(m_pcm);
                if (error == -EAGAIN) {
                    QThread::msleep(10);
                }
            } while (error == -EAGAIN && !m_stopping.load());
            if (error < 0 && !m_stopping.load()) {
                error = snd_pcm_prepare(m_pcm);
            }
        }
        if (m_stopping.load()) {
            return false;
        }
        if (error < 0) {
            fail(error);
            return false;
        }
        return true;
    }

    void AlsaAudioDevice::process() {
        while (!m_stopping.load()) {
            render(m_output.data(), bufferSize());
            convert();
            qint64 position = 0;
            while (position < bufferSize() && !m_stopping.load()) {
                const auto written = snd_pcm_writei(m_pcm, m_nativeOutput.data() + position * m_frameBytes, snd_pcm_uframes_t(bufferSize() - position));
                if (written > 0) {
                    position += written;
                } else if (written == 0 || written == -EAGAIN) {
                    const auto result = snd_pcm_wait(m_pcm, 100);
                    if (result < 0 && !recover(result)) {
                        return;
                    }
                } else if (!recover(int(written))) {
                    return;
                }
            }
        }
    }

    void AlsaAudioDevice::convert() {
        if (m_format == SND_PCM_FORMAT_FLOAT) {
            memcpy(m_nativeOutput.data(), m_output.data(), m_nativeOutput.size());
            return;
        }
        const auto bytes = snd_pcm_format_physical_width(m_format) / 8;
        const auto bits = snd_pcm_format_width(m_format);
        const auto scale = double((uint64_t(1) << (bits - 1)) - 1);
        for (size_t i = 0; i < m_output.size(); ++i) {
            const auto value = std::isfinite(m_output[i]) ? std::clamp(double(m_output[i]), -1.0, 1.0) : 0.0;
            const auto sample = int32_t(std::llround(value * scale));
            auto *destination = m_nativeOutput.data() + i * bytes;
            if (bytes == 2) {
                const auto sample16 = int16_t(sample);
                memcpy(destination, &sample16, 2);
            } else if (bytes == 4) {
                memcpy(destination, &sample, 4);
            } else {
                for (int byte = 0; byte < 3; ++byte) {
                    const auto shift = snd_pcm_format_little_endian(m_format) ? byte * 8 : (2 - byte) * 8;
                    destination[byte] = static_cast<unsigned char>(uint32_t(sample) >> shift);
                }
            }
        }
    }

    void AlsaAudioDevice::updateEndpoint(const AlsaEndpoint &endpoint) {
        if (!endpoint.id.isEmpty()) {
            m_endpoint = endpoint;
            if (!isInitialized()) {
                setIsInitialized(probe());
            }
            return;
        }
        setIsInitialized(false);
        if (isOpen()) {
            close();
            setErrorString(tr("The audio device is no longer available."));
            Q_EMIT closed();
        }
    }

    QString AlsaAudioDevice::streamErrorString(int error) const {
        return QString::fromLocal8Bit(snd_strerror(error));
    }

}

extern "C" Q_DECL_EXPORT talcs::AudioDriver *talcsCreateAudioDriver(QObject *parent) {
    return new talcs::AlsaAudioDriver(parent);
}
