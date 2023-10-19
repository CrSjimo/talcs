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
        QRecursiveMutex mutex;

        RemoteSocket *socket;

        bool remoteIsOpened = false;

        QSharedMemory sharedMemory;
        QVector<float *> sharedAudioData;
        RemoteAudioDevice::ProcessInfo *processInfo = nullptr;
        AudioDataWrapper *buffer = nullptr;

        AudioDeviceCallback *audioDeviceCallback = nullptr;

        RemoteAudioDevice::ProcessInfoCallback callback;

        void remoteOpenRequired(qint64 bufferSize, double sampleRate, const QString &sharedMemoryKey, int maxChannelCount);
        void remoteCloseRequired();
        void remotePrepareBuffer();
    };
}



#endif // TALCS_REMOTEAUDIODEVICE_P_H
