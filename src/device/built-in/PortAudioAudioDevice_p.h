/******************************************************************************
 * Copyright (c) 2023-2025 CrSjimo                                            *
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

#ifndef TALCS_PORTAUDIOAUDIODEVICE_P_H
#define TALCS_PORTAUDIOAUDIODEVICE_P_H

#include <TalcsDevice/private/AudioDevice_p.h>

#include <QMutex>

#include <portaudio.h>

namespace talcs {

    class PortAudioAudioDevicePrivate;
    class PortAudioAudioDriver;

    class PortAudioAudioDevice : public AudioDevice {
        Q_OBJECT
        Q_DECLARE_PRIVATE(PortAudioAudioDevice)
    public:
        ~PortAudioAudioDevice() override;
        bool start(AudioDeviceCallback *audioDeviceCallback) override;
        void stop() override;
        bool open(qint64 bufferSize, double sampleRate) override;
        void close() override;
        void lock() override;
        void unlock() override;
        bool openControlPanel() override;
    protected:
        friend class PortAudioAudioDriver;
        PortAudioAudioDevice(const QString &name, PortAudioAudioDriver *driver);
    };

    class PortAudioAudioDevicePrivate : public AudioDevicePrivate {
        Q_DECLARE_PUBLIC(PortAudioAudioDevice)
    public:
        PaStream *stream = nullptr;
        int deviceIndex = -1;
        int hostApiIndex = -1;
        const PaDeviceInfo *deviceInfo = nullptr;
        AudioDeviceCallback *audioDeviceCallback = nullptr;
        QMutex mutex;
    };

}

#endif // TALCS_PORTAUDIOAUDIODEVICE_P_H
