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

#ifndef TALCS_SOUNDIOAUDIODEVICE_P_H
#define TALCS_SOUNDIOAUDIODEVICE_P_H

#include <QMutex>

#include <soundio/soundio.h>

#include <TalcsDevice/private/AudioDevice_p.h>

namespace talcs {

    class SoundIOAudioDevicePrivate;
    class SoundIOAudioDriver;

    class SoundIOAudioDevice : public AudioDevice {
        Q_OBJECT
        Q_DECLARE_PRIVATE(SoundIOAudioDevice)
    public:
        ~SoundIOAudioDevice() override;
        bool start(AudioDeviceCallback *audioDeviceCallback) override;
        void stop() override;
        bool open(qint64 bufferSize, double sampleRate) override;
        void close() override;
        void lock() override;
        void unlock() override;
        bool openControlPanel() override;

    protected:
        friend class SoundIOAudioDriver;
        SoundIOAudioDevice(const QString &name, SoundIOAudioDriver *driver);

    private slots:
        void handleStreamingError();
        void handleDeviceChanged(); // new slot for device change
    private:
        bool m_isVirtualDefault = false;
        QString m_actualDeviceName; // internal, for real device switching
    };

    class SoundIOAudioDevicePrivate : public AudioDevicePrivate {
        Q_DECLARE_PUBLIC(SoundIOAudioDevice)
    public:
        SoundIoDevice *device = nullptr;
        SoundIoOutStream *outStream = nullptr;
        AudioDeviceCallback *audioDeviceCallback = nullptr;
        QMutex mutex;
        bool isOpen = false;
        
        static void writeCallback(SoundIoOutStream *outStream, int frameCountMin, int frameCountMax);
        static void underflowCallback(SoundIoOutStream *outStream);
        static void errorCallback(SoundIoOutStream *outStream, int err);
    };
}

#endif // TALCS_SOUNDIOAUDIODEVICE_P_H