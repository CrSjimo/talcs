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

#include <cstring>

#include <QDebug>

#include <SDL2/SDL.h>

#include <TalcsCore/AudioSource.h>
#include <TalcsCore/InterleavedAudioDataWrapper.h>

#include "SDLAudioDriver_p.h"

namespace talcs {
    static const QList<double> COMMON_SAMPLE_RATES = {8000,   11025,  12000,  16000,  22050,  24000,
                                                      32000,  44100,  48000,  64000,  88200,  96000,
                                                      128000, 176400, 192000, 256000, 352800, 384000};

    static const QList<qint64> COMMON_SDL_BUFFER_SIZES = {512, 1024, 2048, 4096, 8192};

    /**
     * @class SDLAudioDevice
     * @brief The audio device using SDL2 Audio
     * @see @link URL https://wiki.libsdl.org/ @endlink
     */

    SDLAudioDevice::SDLAudioDevice(const QString &name, SDLAudioDriver *driver)
        : AudioDevice(*new SDLAudioDevicePrivate, driver) {
        Q_D(SDLAudioDevice);
        setName(name);
        setDriver(driver);
        auto devName = name.toUtf8().data();
        SDL_AudioSpec preferredSpec;
        int cnt = SDL_GetNumAudioDevices(0);
        int devIndex = 0;
        for (int i = 0; i < cnt; i++) {
            if (std::strcmp(devName, SDL_GetAudioDeviceName(i, 0)) == 0) {
                devIndex = i;
                break;
            }
        }
        if (SDL_GetAudioDeviceSpec(devIndex, 0, &preferredSpec) != 0) {
            setErrorString(SDL_GetError());
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
    }

    /**
     * Destructor.
     *
     * If the device is not close, it will be closed now.
     */
    SDLAudioDevice::~SDLAudioDevice() {
        SDLAudioDevice::close();
    }

    bool SDLAudioDevice::open(qint64 bufferSize, double sampleRate) {
        Q_D(SDLAudioDevice);
        if (isOpen())
            close();

        d->spec.freq = (int) sampleRate;
        d->spec.format = AUDIO_F32SYS;
        d->spec.channels = (quint8) activeChannelCount();
        d->spec.silence = 0;
        d->spec.samples = (quint16) bufferSize;
        d->spec.callback = [](void *d, quint8 *rawBuf, int length) {
            reinterpret_cast<SDLAudioDevicePrivate *>(d)->sdlCallback(rawBuf, length);
        };
        d->spec.userdata = d;

        d->devId = SDL_OpenAudioDevice(name().toUtf8().data(), 0, &d->spec, nullptr, 0);
        if (d->devId == 0) {
            setErrorString(SDL_GetError());
            return false;
        }
        static_cast<SDLAudioDriver *>(d->driver)->addOpenedDevice(d->devId, this);
        return AudioStreamBase::open(bufferSize, sampleRate);
    }
    void SDLAudioDevice::close() {
        Q_D(SDLAudioDevice);
        SDLAudioDevice::stop();
        SDL_CloseAudioDevice(d->devId);
        static_cast<SDLAudioDriver *>(d->driver)->removeOpenedDevice(d->devId);
        d->devId = 0;
        d->spec = {};
        AudioDevice::close();
    }

    bool SDLAudioDevice::start(AudioDeviceCallback * audioDeviceCallback) {
        Q_D(SDLAudioDevice);
        d->audioDeviceCallback = audioDeviceCallback;
        if (!audioDeviceCallback->deviceWillStartCallback(this))
            return false;
        SDL_PauseAudioDevice(d->devId, 0);
        return AudioDevice::start(audioDeviceCallback);
    }
    void SDLAudioDevice::stop() {
        Q_D(SDLAudioDevice);
        SDL_PauseAudioDevice(d->devId, 1);
        if (d->audioDeviceCallback)
            d->audioDeviceCallback->deviceStoppedCallback();
        d->audioDeviceCallback = nullptr;
        AudioDevice::stop();
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
#ifdef Q_OS_WINDOWS
        return std::system("mmsys.cpl") == 0;
#elif defined(Q_OS_MACOS)
        return std::system("open 'x-apple.systempreferences:com.apple.preference.sound'") == 0;
#else
        return false;
#endif
    }
}
