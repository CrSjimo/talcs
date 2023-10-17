#ifndef TALCS_REMOTEAUDIODEVICE_P_H
#define TALCS_REMOTEAUDIODEVICE_P_H

#include "RemoteAudioDevice.h"

#include <QMutex>
#include <QSharedMemory>

#include "buffer/AudioDataWrapper.h"
#include "device/AudioDevice_p.h"

namespace talcs {
    class RemoteAudioDevicePrivate : public AudioDevicePrivate {
        Q_DECLARE_PUBLIC(RemoteAudioDevice);
        QMutex mutex;

        RemoteSocket *socket;

        bool remoteIsOpened = false;

        QSharedMemory sharedMemory;
        QVector<float *> sharedAudioData;
        AudioDataWrapper *buffer = nullptr;

        AudioDeviceCallback *audioDeviceCallback = nullptr;

        void remoteOpenRequired(qint64 bufferSize, double sampleRate, const QString &sharedMemoryKey, int maxChannelCount);
        void remoteCloseRequired();
        void remotePrepareBuffer();
    };
}



#endif // TALCS_REMOTEAUDIODEVICE_P_H
