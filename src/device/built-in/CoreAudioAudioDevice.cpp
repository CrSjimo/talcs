// SPDX-License-Identifier: LGPL-3.0-or-later

#include "CoreAudioAudioDevice_p.h"

#include <unistd.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>
#include <vector>

#include <QElapsedTimer>
#include <QHash>
#include <QLoggingCategory>
#include <QMutexLocker>
#include <QProcess>
#include <QThread>
#include <QTimer>

#include <TalcsCore/AudioSource.h>
#include <TalcsDevice/private/CoreAudioUtils_p.h>

namespace talcs {

    Q_LOGGING_CATEGORY(lcCoreAudioAudioDevice, "talcs.device.coreaudio.device")

    // Positive counts represent shared streams. A negative count represents a Hog stream.
    static QMutex deviceLeaseMutex;
    static QHash<QString, int> deviceLeases;

    template <class T>
    static OSStatus setHardwareProperty(AudioDeviceID device, AudioObjectPropertySelector selector, const T &value) {
        auto result = coreAudioWriteProperty(device, selector, value);
        if (result != noErr) {
            return result;
        }
        // Hardware changes may complete after AudioObjectSetPropertyData returns.
        QElapsedTimer timer;
        timer.start();
        do {
            T actual{};
            result = coreAudioReadProperty(device, selector, actual);
            if (result != noErr || actual == value) {
                return result;
            }
            QThread::msleep(10);
        } while (timer.elapsed() < 1000);
        return kAudioHardwareIllegalOperationError;
    }

    static bool supportsSampleRate(const std::vector<AudioValueRange> &ranges, double rate) {
        return std::any_of(ranges.cbegin(), ranges.cend(), [rate](const AudioValueRange &range) {
            return rate >= range.mMinimum && rate <= range.mMaximum;
        });
    }

    CoreAudioAudioDevice::CoreAudioAudioDevice(const CoreAudioEndpoint &endpoint, CoreAudioAudioDriver *driver)
        : AudioDevice(*new CoreAudioAudioDevicePrivate, driver) {
        Q_D(CoreAudioAudioDevice);
        setName(endpoint.name);
        setDriver(driver);
        d->endpoint = endpoint;
        d->mode = driver->shareMode();
        d->watchdog = new QTimer(this);
        d->watchdog->setInterval(500);
        connect(d->watchdog, &QTimer::timeout, this, &CoreAudioAudioDevice::handleStreamFailure);
        updateCapabilities();
    }

    CoreAudioAudioDevice::~CoreAudioAudioDevice() {
        CoreAudioAudioDevice::close();
    }

    bool CoreAudioAudioDevice::updateCapabilities() {
        Q_D(CoreAudioAudioDevice);
        setIsInitialized(false);
        const auto fail = [this](OSStatus result) {
            setErrorString(coreAudioErrorString(result));
            qCWarning(lcCoreAudioAudioDevice) << "CoreAudio device capability probe failed." << "device:" << name() << "error:" << errorString();
            return false;
        };
        UInt32 alive = 0;
        auto result = coreAudioReadProperty(d->endpoint.id, kAudioDevicePropertyDeviceIsAlive, alive);
        if (result != noErr || !alive) {
            return fail(result != noErr ? result : kAudioHardwareBadDeviceError);
        }
        QString uid;
        result = coreAudioReadString(d->endpoint.id, kAudioDevicePropertyDeviceUID, uid);
        if (result != noErr || uid != d->endpoint.uid) {
            return fail(result != noErr ? result : kAudioHardwareBadDeviceError);
        }
        result = coreAudioOutputChannelCount(d->endpoint.id, d->endpoint.channels);
        if (result == noErr) {
            result = coreAudioReadProperty(d->endpoint.id, kAudioDevicePropertyNominalSampleRate, d->endpoint.sampleRate);
        }
        if (result == noErr) {
            result = coreAudioReadProperty(d->endpoint.id, kAudioDevicePropertyBufferFrameSize, d->endpoint.bufferSize);
        }
        if (result != noErr || d->endpoint.channels <= 0 || !std::isfinite(d->endpoint.sampleRate) || d->endpoint.sampleRate <= 0 ||
            !d->endpoint.bufferSize || d->endpoint.bufferSize > std::numeric_limits<int>::max()) {
            return fail(result != noErr ? result : kAudioDeviceUnsupportedFormatError);
        }
        QList<double> rates{d->endpoint.sampleRate};
        QList<qint64> sizes{d->endpoint.bufferSize};
        if (d->mode == CoreAudioAudioDriver::Hog) {
            AudioValueRange bufferRange{};
            const auto address = coreAudioPropertyAddress(kAudioDevicePropertyHogMode);
            Boolean settable = false;
            result = AudioObjectIsPropertySettable(d->endpoint.id, &address, &settable);
            if (result != noErr || !settable) {
                return fail(result != noErr ? result : kAudioHardwareUnsupportedOperationError);
            }
            std::vector<AudioValueRange> rateRanges;
            result = coreAudioReadArray(d->endpoint.id, kAudioDevicePropertyAvailableNominalSampleRates, rateRanges);
            if (result == noErr) {
                result = coreAudioReadProperty(d->endpoint.id, kAudioDevicePropertyBufferFrameSizeRange, bufferRange);
            }
            if (result != noErr) {
                return fail(result);
            }
            const double commonRates[] = {8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000, 64000, 88200, 96000, 128000, 176400, 192000, 256000, 352800, 384000};
            for (const auto rate : commonRates) {
                if (supportsSampleRate(rateRanges, rate)) {
                    rates.append(rate);
                }
            }
            for (const auto &range : rateRanges) {
                for (const auto rate : {range.mMinimum, range.mMaximum}) {
                    if (std::isfinite(rate) && rate > 0) {
                        rates.append(rate);
                    }
                }
            }
            if (!std::isfinite(bufferRange.mMinimum) || !std::isfinite(bufferRange.mMaximum) || bufferRange.mMinimum < 1 ||
                bufferRange.mMaximum < bufferRange.mMinimum || bufferRange.mMaximum > std::numeric_limits<int>::max()) {
                return fail(kAudioDeviceUnsupportedFormatError);
            }
            sizes.append(static_cast<qint64>(std::ceil(bufferRange.mMinimum)));
            sizes.append(static_cast<qint64>(std::floor(bufferRange.mMaximum)));
            for (const qint64 frames : {32, 64, 128, 256, 512, 1024, 2048, 4096, 8192}) {
                if (frames >= bufferRange.mMinimum && frames <= bufferRange.mMaximum) {
                    sizes.append(frames);
                }
            }
        }
        std::sort(rates.begin(), rates.end());
        rates.erase(std::unique(rates.begin(), rates.end()), rates.end());
        std::sort(sizes.begin(), sizes.end());
        sizes.erase(std::unique(sizes.begin(), sizes.end()), sizes.end());
        setChannelCount(d->endpoint.channels);
        setAvailableSampleRates(rates);
        setPreferredSampleRate(d->endpoint.sampleRate);
        setAvailableBufferSizes(sizes);
        setPreferredBufferSize(d->endpoint.bufferSize);
        setIsInitialized(true);
        qCDebug(lcCoreAudioAudioDevice) << "CoreAudio device capabilities resolved." << "device:" << name() << "channels:" << channelCount()
                                      << "sample rates:" << rates << "buffer sizes:" << sizes;
        return true;
    }

    bool CoreAudioAudioDevice::open(qint64 bufferSize, double sampleRate) {
        Q_D(CoreAudioAudioDevice);
        close();
        if (!driver() || !driver()->isInitialized()) {
            setErrorString(tr("The audio driver is not initialized."));
            return false;
        }
        if (bufferSize <= 0 || bufferSize > std::numeric_limits<int>::max() || !std::isfinite(sampleRate) || sampleRate <= 0) {
            setErrorString(tr("The requested audio buffer size or sample rate is invalid."));
            return false;
        }
        if (!updateCapabilities()) {
            return false;
        }
        if (activeChannelCount() <= 0) {
            setErrorString(tr("The audio device must have at least one active output channel."));
            return false;
        }
        {
            QMutexLocker locker(&deviceLeaseMutex);
            const int leases = deviceLeases.value(d->endpoint.uid);
            if ((d->mode == CoreAudioAudioDriver::Hog && leases) || leases < 0) {
                setErrorString(tr("The audio device is already in use by this application."));
                return false;
            }
            deviceLeases.insert(d->endpoint.uid, d->mode == CoreAudioAudioDriver::Hog ? -1 : leases + 1);
            d->registered = true;
        }
        qCInfo(lcCoreAudioAudioDevice) << "CoreAudio device open started." << "driver:" << driver()->name() << "device:" << name()
                                     << "buffer size:" << bufferSize << "sample rate:" << sampleRate;
        if (!openStream(bufferSize, sampleRate)) {
            qCWarning(lcCoreAudioAudioDevice) << "CoreAudio device open failed." << "device:" << name() << "error:" << errorString();
            close();
            return false;
        }
        if (d->mode == CoreAudioAudioDriver::Shared) {
            d->outputPointers.resize(activeChannelCount());
        } else {
            d->buffer.resize(activeChannelCount(), bufferSize);
            d->bufferPosition = bufferSize;
        }
        d->streamError.store(noErr);
        d->streamEventTime = coreAudioEventTime();
        setErrorString({});
        AudioStreamBase::open(bufferSize, sampleRate);
        d->watchdog->start();
        qCInfo(lcCoreAudioAudioDevice) << "CoreAudio device open completed." << "driver:" << driver()->name() << "device:" << name()
                                     << "buffer size:" << bufferSize << "sample rate:" << sampleRate;
        return true;
    }

    bool CoreAudioAudioDevice::openStream(qint64 bufferSize, double sampleRate) {
        Q_D(CoreAudioAudioDevice);
        const auto fail = [this](OSStatus result) {
            setErrorString(coreAudioErrorString(result));
            return false;
        };
        const auto id = d->endpoint.id;
        const auto hogAddress = coreAudioPropertyAddress(kAudioDevicePropertyHogMode);
        if (AudioObjectHasProperty(id, &hogAddress)) {
            pid_t owner = -1;
            auto result = coreAudioReadProperty(id, kAudioDevicePropertyHogMode, owner);
            if (result != noErr) {
                return fail(result);
            }
            if (owner != -1) {
                setErrorString(tr("The audio device is in use in Hog mode."));
                return false;
            }
            if (d->mode == CoreAudioAudioDriver::Hog) {
                result = coreAudioWriteProperty(id, kAudioDevicePropertyHogMode, getpid());
                if (result != noErr) {
                    return fail(result);
                }
                d->ownsHog = true;
                result = coreAudioReadProperty(id, kAudioDevicePropertyHogMode, owner);
                if (result != noErr || owner != getpid()) {
                    return fail(result != noErr ? result : kAudioHardwareIllegalOperationError);
                }
            }
        } else if (d->mode == CoreAudioAudioDriver::Hog) {
            return fail(kAudioHardwareUnsupportedOperationError);
        }
        auto result = coreAudioReadProperty(id, kAudioDevicePropertyNominalSampleRate, d->originalSampleRate);
        if (result == noErr) {
            result = coreAudioReadProperty(id, kAudioDevicePropertyBufferFrameSize, d->originalBufferSize);
        }
        if (result != noErr) {
            return fail(result);
        }
        d->hardwareBufferSize = d->originalBufferSize;
        if (d->mode == CoreAudioAudioDriver::Hog) {
            std::vector<AudioValueRange> rates;
            result = coreAudioReadArray(id, kAudioDevicePropertyAvailableNominalSampleRates, rates);
            if (result != noErr || !supportsSampleRate(rates, sampleRate)) {
                return fail(result != noErr ? result : kAudioDeviceUnsupportedFormatError);
            }
            d->sampleRateChanged = sampleRate != d->originalSampleRate;
            if (d->sampleRateChanged) {
                result = setHardwareProperty(id, kAudioDevicePropertyNominalSampleRate, sampleRate);
                if (result != noErr) {
                    return fail(result);
                }
            }
            AudioValueRange range{};
            result = coreAudioReadProperty(id, kAudioDevicePropertyBufferFrameSizeRange, range);
            if (result != noErr || bufferSize < range.mMinimum || bufferSize > range.mMaximum) {
                return fail(result != noErr ? result : kAudioHardwareIllegalOperationError);
            }
            d->bufferSizeChanged = true;
            result = setHardwareProperty(id, kAudioDevicePropertyBufferFrameSize, static_cast<UInt32>(bufferSize));
            if (result != noErr) {
                return fail(result);
            }
            d->hardwareBufferSize = static_cast<UInt32>(bufferSize);
        } else if (sampleRate != d->originalSampleRate) {
            return fail(kAudioDeviceUnsupportedFormatError);
        } else if (bufferSize != d->hardwareBufferSize) {
            setErrorString(tr("The requested buffer size does not match the current hardware buffer size."));
            return false;
        }
        result = coreAudioOutputChannelCount(id, d->endpoint.channels);
        if (result != noErr || d->endpoint.channels <= 0) {
            return fail(result != noErr ? result : kAudioDeviceUnsupportedFormatError);
        }
        setChannelCount(d->endpoint.channels);
        AudioComponentDescription description{};
        description.componentType = kAudioUnitType_Output;
        description.componentSubType = kAudioUnitSubType_HALOutput;
        description.componentManufacturer = kAudioUnitManufacturer_Apple;
        const auto component = AudioComponentFindNext(nullptr, &description);
        if (!component) {
            return fail(kAudioHardwareUnsupportedOperationError);
        }
        result = AudioComponentInstanceNew(component, &d->unit);
        if (result != noErr) {
            return fail(result);
        }
        const UInt32 enable = 1;
        const UInt32 disable = 0;
        result = AudioUnitSetProperty(d->unit, kAudioOutputUnitProperty_EnableIO, kAudioUnitScope_Output, 0, &enable, sizeof(enable));
        if (result == noErr) {
            result = AudioUnitSetProperty(d->unit, kAudioOutputUnitProperty_EnableIO, kAudioUnitScope_Input, 1, &disable, sizeof(disable));
        }
        if (result == noErr) {
            result = AudioUnitSetProperty(d->unit, kAudioOutputUnitProperty_CurrentDevice, kAudioUnitScope_Global, 0, &id, sizeof(id));
        }
        if (result != noErr) {
            return fail(result);
        }
        AudioStreamBasicDescription format{};
        format.mSampleRate = sampleRate;
        format.mFormatID = kAudioFormatLinearPCM;
        format.mFormatFlags = kAudioFormatFlagsNativeFloatPacked | kAudioFormatFlagIsNonInterleaved;
        format.mBytesPerPacket = sizeof(float);
        format.mFramesPerPacket = 1;
        format.mBytesPerFrame = sizeof(float);
        format.mChannelsPerFrame = d->endpoint.channels;
        format.mBitsPerChannel = sizeof(float) * 8;
        result = AudioUnitSetProperty(d->unit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &format, sizeof(format));
        UInt32 maxFrames = d->hardwareBufferSize;
        UInt32 variableFrames = 0;
        coreAudioReadProperty(id, kAudioDevicePropertyUsesVariableBufferFrameSizes, variableFrames);
        maxFrames = std::max(maxFrames, variableFrames);
        AudioValueRange range{};
        if (coreAudioReadProperty(id, kAudioDevicePropertyBufferFrameSizeRange, range) == noErr &&
            std::isfinite(range.mMaximum) && range.mMaximum >= maxFrames && range.mMaximum <= std::numeric_limits<UInt32>::max()) {
            maxFrames = static_cast<UInt32>(std::ceil(range.mMaximum));
        }
        if (result == noErr) {
            result = AudioUnitSetProperty(d->unit, kAudioUnitProperty_MaximumFramesPerSlice, kAudioUnitScope_Global, 0, &maxFrames, sizeof(maxFrames));
        }
        const AURenderCallbackStruct callback{CoreAudioAudioDevicePrivate::render, d};
        if (result == noErr) {
            result = AudioUnitSetProperty(d->unit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, 0, &callback, sizeof(callback));
        }
        if (result == noErr) {
            result = AudioUnitInitialize(d->unit);
        }
        if (result != noErr) {
            return fail(result);
        }
        double actualRate = 0;
        result = coreAudioReadProperty(id, kAudioDevicePropertyNominalSampleRate, actualRate);
        if (result != noErr || actualRate != sampleRate) {
            return fail(result != noErr ? result : kAudioDeviceUnsupportedFormatError);
        }
        UInt32 actualBufferSize = 0;
        result = coreAudioReadProperty(id, kAudioDevicePropertyBufferFrameSize, actualBufferSize);
        if (result != noErr || actualBufferSize != d->hardwareBufferSize) {
            return fail(result != noErr ? result : kAudioHardwareIllegalOperationError);
        }
        return true;
    }

    void CoreAudioAudioDevice::close() {
        Q_D(CoreAudioAudioDevice);
        d->watchdog->stop();
        stop();
        if (d->unit) {
            AudioUnitUninitialize(d->unit);
            const auto result = AudioComponentInstanceDispose(d->unit);
            if (result != noErr) {
                qCWarning(lcCoreAudioAudioDevice) << "CoreAudio audio unit disposal failed." << "device:" << name() << "error:" << coreAudioErrorString(result);
            }
            d->unit = nullptr;
        }
        if (d->ownsHog) {
            pid_t owner = -1;
            QString uid;
            auto result = coreAudioReadString(d->endpoint.id, kAudioDevicePropertyDeviceUID, uid);
            if (result == noErr && uid == d->endpoint.uid) {
                result = coreAudioReadProperty(d->endpoint.id, kAudioDevicePropertyHogMode, owner);
            }
            if (result == noErr && uid == d->endpoint.uid && owner == getpid()) {
                const auto restore = [this, d](AudioObjectPropertySelector selector, auto value) {
                    const auto result = setHardwareProperty(d->endpoint.id, selector, value);
                    if (result != noErr) {
                        qCWarning(lcCoreAudioAudioDevice) << "CoreAudio hardware setting restoration failed." << "device:" << name()
                                                        << "property:" << selector << "error:" << coreAudioErrorString(result);
                    }
                };
                if (d->sampleRateChanged) {
                    restore(kAudioDevicePropertyNominalSampleRate, d->originalSampleRate);
                }
                if (d->bufferSizeChanged || d->sampleRateChanged) {
                    restore(kAudioDevicePropertyBufferFrameSize, d->originalBufferSize);
                }
                // Setting Hog mode toggles ownership, regardless of the supplied PID.
                result = coreAudioWriteProperty(d->endpoint.id, kAudioDevicePropertyHogMode, pid_t(-1));
                if (result == noErr) {
                    result = coreAudioReadProperty(d->endpoint.id, kAudioDevicePropertyHogMode, owner);
                    if (result == noErr && owner == getpid()) {
                        result = kAudioHardwareIllegalOperationError;
                    }
                }
            }
            if (result == noErr || result == kAudioHardwareBadObjectError || result == kAudioHardwareBadDeviceError) {
                d->ownsHog = false;
            } else {
                qCWarning(lcCoreAudioAudioDevice) << "CoreAudio Hog mode release failed." << "device:" << name() << "error:" << coreAudioErrorString(result);
            }
        }
        if (d->registered && !d->ownsHog) {
            QMutexLocker locker(&deviceLeaseMutex);
            const auto it = deviceLeases.find(d->endpoint.uid);
            if (it != deviceLeases.end()) {
                if (it.value() <= 1) {
                    deviceLeases.erase(it);
                } else {
                    --it.value();
                }
            }
            d->registered = false;
        }
        if (!d->ownsHog) {
            d->sampleRateChanged = false;
            d->bufferSizeChanged = false;
        }
        d->hardwareBufferSize = 0;
        d->outputBuffer.reset(nullptr, 0, 0);
        d->outputPointers.clear();
        d->buffer.resize(0, 0);
        AudioDevice::close();
    }

    bool CoreAudioAudioDevice::start(AudioDeviceCallback *callback) {
        Q_D(CoreAudioAudioDevice);
        if (!isOpen() || !callback) {
            setErrorString(tr("The audio device must be open before playback starts."));
            return false;
        }
        stop();
        if (!callback->deviceWillStartCallback(this)) {
            return false;
        }
        d->streamError.store(noErr);
        d->streamEventTime = coreAudioEventTime();
        {
            QMutexLocker locker(&d->callbackMutex);
            if (d->mode == CoreAudioAudioDriver::Hog) {
                d->buffer.clear();
                d->bufferPosition = d->buffer.sampleCount();
            }
            d->callback = callback;
        }
        AudioDevice::start(callback);
        const auto result = AudioOutputUnitStart(d->unit);
        if (result != noErr) {
            setErrorString(coreAudioErrorString(result));
            qCWarning(lcCoreAudioAudioDevice) << "CoreAudio playback start failed." << "device:" << name() << "error:" << errorString();
            stop();
            return false;
        }
        setErrorString({});
        qCInfo(lcCoreAudioAudioDevice) << "CoreAudio playback started." << "device:" << name();
        return true;
    }

    void CoreAudioAudioDevice::stop() {
        Q_D(CoreAudioAudioDevice);
        if (d->unit && isStarted()) {
            const auto result = AudioOutputUnitStop(d->unit);
            if (result != noErr) {
                qCWarning(lcCoreAudioAudioDevice) << "CoreAudio playback stop failed." << "device:" << name() << "error:" << coreAudioErrorString(result);
            } else {
                const auto resetResult = AudioUnitReset(d->unit, kAudioUnitScope_Global, 0);
                if (resetResult != noErr) {
                    qCWarning(lcCoreAudioAudioDevice) << "CoreAudio audio unit reset failed." << "device:" << name() << "error:" << coreAudioErrorString(resetResult);
                }
            }
        }
        AudioDeviceCallback *callback = nullptr;
        {
            QMutexLocker locker(&d->callbackMutex);
            callback = d->callback;
            d->callback = nullptr;
        }
        AudioDevice::stop();
        d->streamError.store(noErr);
        if (callback) {
            callback->deviceStoppedCallback();
        }
    }

    void CoreAudioAudioDevice::lock() {
        Q_D(CoreAudioAudioDevice);
        d->callbackMutex.lock();
    }

    void CoreAudioAudioDevice::unlock() {
        Q_D(CoreAudioAudioDevice);
        d->callbackMutex.unlock();
    }

    OSStatus CoreAudioAudioDevicePrivate::render(void *context, AudioUnitRenderActionFlags *flags, const AudioTimeStamp *, UInt32, UInt32 frames, AudioBufferList *data) {
        auto *d = static_cast<CoreAudioAudioDevicePrivate *>(context);
        if (!data) {
            d->streamError.store(kAudioHardwareBadPropertySizeError);
            return kAudioHardwareBadPropertySizeError;
        }
        for (UInt32 i = 0; i < data->mNumberBuffers; ++i) {
            if (data->mBuffers[i].mData) {
                std::memset(data->mBuffers[i].mData, 0, data->mBuffers[i].mDataByteSize);
            }
        }
        QMutexLocker locker(&d->callbackMutex);
        if (!d->callback || d->streamError.load() != noErr || !frames) {
            *flags |= kAudioUnitRenderAction_OutputIsSilence;
            return noErr;
        }
        bool valid = data->mNumberBuffers == static_cast<UInt32>(d->endpoint.channels);
        for (UInt32 i = 0; valid && i < data->mNumberBuffers; ++i) {
            const auto &buffer = data->mBuffers[i];
            valid = buffer.mData && buffer.mNumberChannels == 1 && quint64(buffer.mDataByteSize) >= quint64(frames) * sizeof(float);
        }
        if (!valid) {
            d->streamError.store(kAudioHardwareBadPropertySizeError);
            *flags |= kAudioUnitRenderAction_OutputIsSilence;
            return noErr;
        }
        if (d->mode == CoreAudioAudioDriver::Shared) {
            const int channels = static_cast<int>(d->outputPointers.size());
            for (int channel = 0; channel < channels; ++channel) {
                d->outputPointers[channel] = static_cast<float *>(data->mBuffers[channel].mData);
            }
            d->outputBuffer.reset(d->outputPointers.constData(), channels, frames);
            d->callback->workCallback(&d->outputBuffer);
            for (auto *output : d->outputPointers) {
                for (UInt32 frame = 0; frame < frames; ++frame) {
                    if (!std::isfinite(output[frame])) {
                        output[frame] = 0;
                    }
                }
            }
            d->outputBuffer.reset(nullptr, 0, 0);
            return noErr;
        }
        UInt32 position = 0;
        while (position < frames) {
            if (d->bufferPosition == d->buffer.sampleCount()) {
                d->buffer.clear();
                d->callback->workCallback(&d->buffer);
                d->bufferPosition = 0;
            }
            const auto count = std::min<qint64>(frames - position, d->buffer.sampleCount() - d->bufferPosition);
            for (int channel = 0; channel < d->buffer.channelCount(); ++channel) {
                const auto *source = d->buffer.constData(channel) + d->bufferPosition;
                auto *destination = static_cast<float *>(data->mBuffers[channel].mData) + position;
                for (qint64 i = 0; i < count; ++i) {
                    destination[i] = std::isfinite(source[i]) ? source[i] : 0;
                }
            }
            d->bufferPosition += count;
            position += static_cast<UInt32>(count);
        }
        return noErr;
    }

    bool CoreAudioAudioDevice::openControlPanel() {
        QProcess process;
        process.setProgram(QStringLiteral("/usr/bin/open"));
        process.setArguments({QStringLiteral("-b"), QStringLiteral("com.apple.audio.AudioMIDISetup")});
        if (!process.startDetached()) {
            qCWarning(lcCoreAudioAudioDevice) << "CoreAudio control panel launch failed." << "device:" << name() << "error:" << process.errorString();
            return false;
        }
        return true;
    }

    void CoreAudioAudioDevice::handleEndpointChange(const QList<CoreAudioEndpoint> &endpoints, const QHash<AudioDeviceID, qint64> &failedDevices) {
        Q_D(CoreAudioAudioDevice);
        const auto endpoint = std::find_if(endpoints.cbegin(), endpoints.cend(), [d](const CoreAudioEndpoint &endpoint) {
            return endpoint.uid == d->endpoint.uid;
        });
        const bool available = endpoint != endpoints.cend();
        const bool invalidated = !available || (available && (endpoint->id != d->endpoint.id || endpoint->channels != channelCount() ||
            endpoint->sampleRate != sampleRate() || endpoint->bufferSize != d->hardwareBufferSize ||
            (d->ownsHog ? endpoint->hogOwner != getpid() : endpoint->hogOwner != -1)));
        const bool streamFailed = failedDevices.value(d->endpoint.id) >= d->streamEventTime;
        const bool wasClosed = isOpen() && (invalidated || streamFailed);
        if (wasClosed) {
            setErrorString(tr("The audio device configuration changed or the audio stream stopped unexpectedly."));
            qCWarning(lcCoreAudioAudioDevice) << "CoreAudio device is closing after an endpoint change." << "device:" << name() << "available:" << available;
            close();
        }
        if (available) {
            setName(endpoint->name);
            if (!isOpen()) {
                d->endpoint = *endpoint;
                const auto error = errorString();
                updateCapabilities();
                if (wasClosed) {
                    setErrorString(error);
                }
            }
        } else {
            setIsInitialized(false);
        }
        if (wasClosed) {
            Q_EMIT closed();
        }
    }

    void CoreAudioAudioDevice::handleStreamFailure() {
        Q_D(CoreAudioAudioDevice);
        if (!isOpen()) {
            return;
        }
        auto result = d->streamError.load();
        UInt32 alive = 0;
        if (result == noErr) {
            result = coreAudioReadProperty(d->endpoint.id, kAudioDevicePropertyDeviceIsAlive, alive);
            if (result == noErr && !alive) {
                result = kAudioHardwareBadDeviceError;
            }
        }
        if (result == noErr && isStarted()) {
            OSStatus renderError = noErr;
            UInt32 errorSize = sizeof(renderError);
            result = AudioUnitGetProperty(d->unit, kAudioUnitProperty_LastRenderError, kAudioUnitScope_Global, 0, &renderError, &errorSize);
            if (result == noErr) {
                result = renderError;
            }
        }
        if (result == noErr && isStarted()) {
            UInt32 running = 0;
            UInt32 size = sizeof(running);
            result = AudioUnitGetProperty(d->unit, kAudioOutputUnitProperty_IsRunning, kAudioUnitScope_Global, 0, &running, &size);
            if (result == noErr && !running) {
                result = kAudioHardwareIllegalOperationError;
            }
        }
        if (result == noErr) {
            return;
        }
        setErrorString(coreAudioErrorString(result));
        qCWarning(lcCoreAudioAudioDevice) << "CoreAudio audio stream failed." << "device:" << name() << "error:" << errorString();
        close();
        if (driver()) {
            static_cast<CoreAudioAudioDriver *>(driver())->requestRefresh();
        }
        Q_EMIT closed();
    }

}
