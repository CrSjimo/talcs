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

#ifndef TALCS_REMOTEAUDIODEVICE_H
#define TALCS_REMOTEAUDIODEVICE_H

#include <TalcsDevice/AudioDevice.h>
#include <TalcsRemote/RemoteProcessInfo.h>

namespace talcs {

    class RemoteAudioDevicePrivate;
    class RemoteSocket;

    class TALCSREMOTE_EXPORT RemoteAudioDevice : public AudioDevice {
        Q_OBJECT
        Q_DECLARE_PRIVATE(RemoteAudioDevice)
    public:
        explicit RemoteAudioDevice(RemoteSocket *socket, const QString &name, QObject *parent = nullptr);
        ~RemoteAudioDevice() override;

        bool open(qint64 bufferSize, double sampleRate) override;
        void close() override;

        void addProcessInfoCallback(RemoteProcessInfoCallback *callback);
        void removeProcessInfoCallback(RemoteProcessInfoCallback *callback);

        bool start(AudioDeviceCallback *audioDeviceCallback) override;
        void stop() override;

        void lock() override;
        void unlock() override;

    signals:
        void remoteOpened(qint64 bufferSize, double sampleRate, int maxChannelCount);
    };

}

#endif // TALCS_REMOTEAUDIODEVICE_H
