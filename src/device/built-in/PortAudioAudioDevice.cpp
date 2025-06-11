#include "PortAudioAudioDevice_p.h"

#include <QDebug>

#ifdef Q_OS_WIN
#include <pa_win_wasapi.h>
#endif

#include <mutex>
#include <TalcsDevice/private/PortAudioAudioDriver_p.h>
#include <TalcsCore/InterleavedAudioDataWrapper.h>
#include <TalcsCore/AudioSource.h>

namespace talcs {

    static const QList<double> COMMON_SAMPLE_RATES = {8000,   11025,  12000,  16000,  22050,  24000,
                                                      32000,  44100,  48000,  64000,  88200,  96000,
                                                      128000, 176400, 192000, 256000, 352800, 384000};

    PortAudioAudioDevice::PortAudioAudioDevice(const QString &name, PortAudioAudioDriver *driver)
        : AudioDevice(*new PortAudioAudioDevicePrivate, driver) {
        Q_D(PortAudioAudioDevice);
        setName(name);
        setDriver(driver);
        d->stream = nullptr;
        d->deviceIndex = -1;
        d->hostApiIndex = driver->d_func()->hostApiIndex;
        d->deviceInfo = nullptr;
        d->audioDeviceCallback = nullptr;
        d->channelCount = 2;
        int deviceCount = Pa_GetDeviceCount();
        for (int i = 0; i < deviceCount; ++i) {
            const PaDeviceInfo *info = Pa_GetDeviceInfo(i);
            if (info && QString::fromUtf8(info->name) == name && info->hostApi == d->hostApiIndex) {
                d->deviceIndex = i;
                d->deviceInfo = info;
                break;
            }
        }
        if (d->deviceIndex < 0 || !d->deviceInfo) {
            setErrorString("PortAudio: Device not found: " + name);
            setIsInitialized(false);
            return;
        }
        int ch = d->deviceInfo->maxOutputChannels;
        setChannelCount(ch);
        setPreferredBufferSize(1024);
        setAvailableBufferSizes({512, 1024, 2048, 4096, 8192});
        setPreferredSampleRate(d->deviceInfo->defaultSampleRate);
        QList<double> availableSampleRates;
        for (double rate : COMMON_SAMPLE_RATES) {
            PaStreamParameters outputParams = {};
            outputParams.device = d->deviceIndex;
            outputParams.channelCount = d->channelCount;
            outputParams.sampleFormat = paFloat32;
            outputParams.suggestedLatency = d->deviceInfo->defaultLowOutputLatency;
            outputParams.hostApiSpecificStreamInfo = nullptr;
#ifdef Q_OS_WIN
            if (Pa_GetHostApiInfo(d->deviceInfo->hostApi)->type == paWASAPI && driver->d_func()->wasapiExclusive) {
            static PaWasapiStreamInfo wasapiInfo = {};
            wasapiInfo.size = sizeof(PaWasapiStreamInfo);
            wasapiInfo.hostApiType = paWASAPI;
            wasapiInfo.version = 1;
            wasapiInfo.flags = paWinWasapiExclusive;
            outputParams.hostApiSpecificStreamInfo = &wasapiInfo;
            }
#endif
            PaError err = Pa_IsFormatSupported(nullptr, &outputParams, rate);
            if (err == paFormatIsSupported) {
            availableSampleRates.append(rate);
            }
        }
        setAvailableSampleRates(availableSampleRates);
        setIsInitialized(true);
    }

    PortAudioAudioDevice::~PortAudioAudioDevice() {
        PortAudioAudioDevice::close();
    }

    static int portaudioCallback(const void *input, void *output, unsigned long frameCount, const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData) {
        auto *d = static_cast<PortAudioAudioDevicePrivate *>(userData);
        if (!d || !d->audioDeviceCallback || !output) {
            return paContinue;
        }
        auto out = static_cast<float *>(output);
        InterleavedAudioDataWrapper buf(out, d->channelCount, frameCount);
        buf.clear();
        QMutexLocker lock(&d->mutex);
        d->audioDeviceCallback->workCallback(&buf);
        return paContinue;
    }

    bool PortAudioAudioDevice::open(qint64 bufferSize, double sampleRate) {
        Q_D(PortAudioAudioDevice);
        if (!isInitialized() || d->deviceIndex < 0 || !d->deviceInfo)
            return false;
        if (d->stream) {
            close();
        }
        d->channelCount = channelCount();
        PaStreamParameters outputParams = {};
        outputParams.device = d->deviceIndex;
        outputParams.channelCount = d->channelCount;
        outputParams.sampleFormat = paFloat32;
        outputParams.suggestedLatency = d->deviceInfo->defaultLowOutputLatency;
        outputParams.hostApiSpecificStreamInfo = nullptr;
#ifdef Q_OS_WIN
        if (Pa_GetHostApiInfo(d->deviceInfo->hostApi)->type == paWASAPI && static_cast<PortAudioAudioDriver *>(driver())->d_func()->wasapiExclusive) {
            static PaWasapiStreamInfo wasapiInfo = {};
            wasapiInfo.size = sizeof(PaWasapiStreamInfo);
            wasapiInfo.hostApiType = paWASAPI;
            wasapiInfo.version = 1;
            wasapiInfo.flags = paWinWasapiExclusive;
            outputParams.hostApiSpecificStreamInfo = &wasapiInfo;
        }
#endif
        PaError err = Pa_OpenStream(&d->stream, nullptr, &outputParams, sampleRate, bufferSize, paNoFlag, portaudioCallback, d);
        if (err != paNoError) {
            setErrorString(QString::fromUtf8(Pa_GetErrorText(err)));
            d->stream = nullptr;
            return false;
        }
        return AudioStreamBase::open(bufferSize, sampleRate);
    }

    void PortAudioAudioDevice::close() {
        Q_D(PortAudioAudioDevice);
        if (d->stream) {
            Pa_CloseStream(d->stream);
            d->stream = nullptr;
        }
        AudioDevice::close();
    }

    bool PortAudioAudioDevice::start(AudioDeviceCallback *audioDeviceCallback) {
        Q_D(PortAudioAudioDevice);
        d->audioDeviceCallback = audioDeviceCallback;
        if (!d->stream)
            return false;
        PaError err = Pa_StartStream(d->stream);
        if (err != paNoError) {
            setErrorString(QString::fromUtf8(Pa_GetErrorText(err)));
            return false;
        }
        return AudioDevice::start(audioDeviceCallback);
    }

    void PortAudioAudioDevice::stop() {
        Q_D(PortAudioAudioDevice);
        if (d->stream) {
            Pa_StopStream(d->stream);
        }
        d->audioDeviceCallback = nullptr;
        AudioDevice::stop();
    }

    void PortAudioAudioDevice::lock() {
        Q_D(PortAudioAudioDevice);
        d->mutex.lock();
    }

    void PortAudioAudioDevice::unlock() {
        Q_D(PortAudioAudioDevice);
        d->mutex.unlock();
    }

    bool PortAudioAudioDevice::openControlPanel() {
#ifdef Q_OS_WINDOWS
        return std::system("mmsys.cpl") == 0;
#elif defined(Q_OS_MACOS)
        return std::system("open 'x-apple.systempreferences:com.apple.preference.sound'") == 0;
#else
        return false;
#endif
    }

} // namespace talcs
