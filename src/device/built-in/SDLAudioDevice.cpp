/******************************************************************************
 * Copyright (c) 2023 CrSjimo                                                 *
 *                                                                            *
 * This file is part of TALCS.                                                *
 *                                                                            *
 * TALCS is free software: you can redistribute it and/or modify it under the *
 * terms of the GNU Lesser General Public License as published by the Free    *
 * Software Foundation, either version 3 of the License, or (at your option)  *
 * any later version.                                                         *
 *                                                                            *
 * TALCS is distributed in the hope that it will be useful, but WITHOUT ANY   *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS  *
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for    *
 * more details.                                                              *
 *                                                                            *
 * You should have received a copy of the GNU Lesser General Public License   *
 * along with TALCS. If not, see <https://www.gnu.org/licenses/>.             *
 ******************************************************************************/

#include "SDLAudioDevice_p.h"

#include <QDebug>
#include <QLoggingCategory>

#include <SDL2/SDL.h>

#include <TalcsCore/AudioSource.h>
#include <TalcsCore/InterleavedAudioDataWrapper.h>

#include <TalcsDevice/private/SDLAudioDriver_p.h>

namespace talcs {

    Q_LOGGING_CATEGORY(lcSDLAudioDevice, "talcs.device.sdl.device")

    static const QList<double> COMMON_SAMPLE_RATES = {8000,   11025,  12000,  16000,  22050,  24000,
                                                      32000,  44100,  48000,  64000,  88200,  96000,
                                                      128000, 176400, 192000, 256000, 352800, 384000};

    static const QList<qint64> COMMON_SDL_BUFFER_SIZES = {512, 1024, 2048, 4096, 8192};

    SDLAudioDevice::SDLAudioDevice(const QString &name, SDLAudioDriver *driver)
        : AudioDevice(*new SDLAudioDevicePrivate, driver) {
        setName(name);
        setDriver(driver);
        qCInfo(lcSDLAudioDevice) << "SDL audio device initialization started." << "driver:" << driver->name() << "device:" << name;
        SDL_AudioSpec preferredSpec{};
        const int count = SDL_GetNumAudioDevices(0);
        if (count < 0) {
            qCWarning(lcSDLAudioDevice) << "SDL audio device enumeration failed during initialization." << "device:" << name << "error:" << SDL_GetError();
        }
        int deviceIndex = -1;
        const auto deviceName = name.toUtf8();
        for (int i = 0; i < count; ++i) {
            if (deviceName == SDL_GetAudioDeviceName(i, 0)) {
                deviceIndex = i;
                break;
            }
        }
        if (deviceIndex < 0) {
            setErrorString(tr("The audio device is no longer available."));
            qCWarning(lcSDLAudioDevice) << "SDL audio device initialization failed." << "device:" << name << "error:" << errorString();
            return;
        }
        if (SDL_GetAudioDeviceSpec(deviceIndex, 0, &preferredSpec) != 0) {
            setErrorString(SDL_GetError());
            qCWarning(lcSDLAudioDevice) << "SDL audio device format retrieval failed." << "device:" << name << "error:" << errorString();
            return;
        }
        setChannelCount(qMin(quint8(8), preferredSpec.channels));
        if (preferredSpec.samples == 0)
            preferredSpec.samples = 1024;
        setPreferredBufferSize(preferredSpec.samples);
        setAvailableBufferSizes(COMMON_SDL_BUFFER_SIZES);
        if (preferredSpec.freq == 0)
            preferredSpec.freq = 48000;
        setPreferredSampleRate(preferredSpec.freq);
        setAvailableSampleRates(COMMON_SAMPLE_RATES);

        setIsInitialized(true);
        qCDebug(lcSDLAudioDevice) << "SDL audio device capabilities resolved." << "device:" << name << "channels:" << channelCount()
                                << "preferred buffer size:" << preferredBufferSize() << "preferred sample rate:" << preferredSampleRate()
                                << "buffer sizes:" << availableBufferSizes() << "sample rates:" << availableSampleRates();
    }

    SDLAudioDevice::~SDLAudioDevice() {
        SDLAudioDevice::close();
    }

    bool SDLAudioDevice::open(qint64 bufferSize, double sampleRate) {
        Q_D(SDLAudioDevice);
        qCInfo(lcSDLAudioDevice) << "SDL audio device open started." << "driver:" << driver()->name() << "device:" << name()
                               << "buffer size:" << bufferSize << "sample rate:" << sampleRate << "channels:" << activeChannelCount();
        if (isOpen())
            close();

        d->spec.freq = static_cast<int>(sampleRate);
        d->spec.format = AUDIO_F32SYS;
        d->spec.channels = static_cast<quint8>(activeChannelCount());
        d->spec.silence = 0;
        d->spec.samples = static_cast<quint16>(bufferSize);
        d->spec.callback = [](void *d, quint8 *rawBuf, int length) {
            static_cast<SDLAudioDevicePrivate *>(d)->sdlCallback(rawBuf, length);
        };
        d->spec.userdata = d;

        d->devId = SDL_OpenAudioDevice(name().toUtf8().constData(), 0, &d->spec, nullptr, 0);
        if (d->devId == 0) {
            setErrorString(SDL_GetError());
            qCWarning(lcSDLAudioDevice) << "SDL audio device open failed." << "device:" << name()
                                      << "buffer size:" << bufferSize << "sample rate:" << sampleRate << "error:" << errorString();
            return false;
        }
        static_cast<SDLAudioDriver *>(d->driver.data())->addOpenedDevice(d->devId, this);
        qCInfo(lcSDLAudioDevice) << "SDL audio device open completed." << "device:" << name() << "device ID:" << d->devId;
        return AudioStreamBase::open(bufferSize, sampleRate);
    }
    void SDLAudioDevice::close() {
        Q_D(SDLAudioDevice);
        const bool wasOpen = isOpen();
        SDLAudioDevice::stop();
        SDL_CloseAudioDevice(d->devId);
        if (d->driver)
            static_cast<SDLAudioDriver *>(d->driver.data())->removeOpenedDevice(d->devId);
        d->devId = 0;
        d->spec = {};
        AudioDevice::close();
        if (wasOpen) {
            qCInfo(lcSDLAudioDevice) << "SDL audio device closed." << "device:" << name();
        }
    }

    bool SDLAudioDevice::start(AudioDeviceCallback * audioDeviceCallback) {
        Q_D(SDLAudioDevice);
        qCInfo(lcSDLAudioDevice) << "SDL audio playback start requested." << "device:" << name() << "device ID:" << d->devId;
        d->audioDeviceCallback = audioDeviceCallback;
        if (!audioDeviceCallback->deviceWillStartCallback(this)) {
            qCWarning(lcSDLAudioDevice) << "SDL audio playback preparation callback failed." << "device:" << name();
            return false;
        }
        SDL_PauseAudioDevice(d->devId, 0);
        qCInfo(lcSDLAudioDevice) << "SDL audio playback started." << "device:" << name();
        return AudioDevice::start(audioDeviceCallback);
    }
    void SDLAudioDevice::stop() {
        Q_D(SDLAudioDevice);
        const bool wasStarted = isStarted();
        SDL_PauseAudioDevice(d->devId, 1);
        if (d->audioDeviceCallback)
            d->audioDeviceCallback->deviceStoppedCallback();
        d->audioDeviceCallback = nullptr;
        AudioDevice::stop();
        if (wasStarted) {
            qCInfo(lcSDLAudioDevice) << "SDL audio playback stopped." << "device:" << name();
        }
    }
    void SDLAudioDevice::lock() {
        Q_D(SDLAudioDevice);
        SDL_LockAudioDevice(d->devId);
    }
    void SDLAudioDevice::unlock() {
        Q_D(SDLAudioDevice);
        SDL_UnlockAudioDevice(d->devId);
    }

    void SDLAudioDevicePrivate::sdlCallback(quint8 * rawBuf, int length) {
        Q_Q(SDLAudioDevice);
        SDL_memset(rawBuf, 0, length);
        InterleavedAudioDataWrapper buf(reinterpret_cast<float *>(rawBuf), spec.channels, length / spec.channels / 4);
        audioDeviceCallback->workCallback(&buf);
    }

    bool SDLAudioDevice::openControlPanel() {
#ifdef Q_OS_MACOS
        const auto result = std::system("open 'x-apple.systempreferences:com.apple.preference.sound'");
        if (result != 0) {
            qCWarning(lcSDLAudioDevice) << "SDL audio control panel launch failed." << "device:" << name() << "process status:" << result;
        }
        return result == 0;
#else
        return false;
#endif
    }
}
