//
// Created by Crs_1 on 2023/10/9.
//

#ifndef DIFFSCOPE_REMOTEAUDIODEVICE_H
#define DIFFSCOPE_REMOTEAUDIODEVICE_H

#include "device/AudioDevice.h"

namespace talcs {

    class RemoteAudioDevicePrivate;
    class RemoteSocket;

    class TALCS_EXPORT RemoteAudioDevice : public AudioDevice {
        Q_OBJECT
        Q_DECLARE_PRIVATE(RemoteAudioDevice)
    public:
        explicit RemoteAudioDevice(RemoteSocket *socket, const QString &name, QObject *parent = nullptr);
        ~RemoteAudioDevice() override;

        bool open(qint64 bufferSize, double sampleRate) override;
        void close() override;

        bool start(AudioDeviceCallback *audioDeviceCallback) override;
        void stop() override;

        void lock() override;
        void unlock() override;

    signals:
        void remoteOpened(qint64 bufferSize, double sampleRate, int maxChannelCount);
    };

} // talcs

#endif // DIFFSCOPE_REMOTEAUDIODEVICE_H
