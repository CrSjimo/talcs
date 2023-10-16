//
// Created by Crs_1 on 2023/10/9.
//

#include "RemoteAudioDevice.h"
#include "RemoteAudioDevice_p.h"

#include "RemoteSocket.h"
#include "source/AudioSource.h"

namespace talcs {

    RemoteAudioDevice::RemoteAudioDevice(RemoteSocket *socket, const QString &name, QObject *parent)
        : AudioDevice(*new RemoteAudioDevicePrivate, parent) {
        Q_D(RemoteAudioDevice);
        d->socket = socket;
        setName(name);
        setDriver(nullptr);
        d->socket->bind("audio", "openRequired",
                        [d](qint64 bufferSize, double sampleRate, const std::string &sharedMemoryKey, int maxBusCount) {
                            d->remoteOpenRequired(bufferSize, sampleRate, sharedMemoryKey.c_str(), maxBusCount);
                        });
        d->socket->bind("audio", "closeRequired", [d] { d->remoteCloseRequired(); });
        d->socket->bind("audio", "prepareBuffer", [d]() { d->remotePrepareBuffer(); });
        connect(socket, &RemoteSocket::socketStatusChanged, this, [=](int newStatus) {
            if (newStatus != talcs::RemoteSocket::Connected && this->isOpen()) {
                this->close();
            }
        });
    }

    RemoteAudioDevice::~RemoteAudioDevice() {
        Q_D(RemoteAudioDevice);
        RemoteAudioDevice::close();
        d->socket->unbind("audio", "openRequired");
        d->socket->unbind("audio", "closeRequired");
        d->socket->unbind("audio", "prepareBuffer");
    }

    void RemoteAudioDevicePrivate::remoteOpenRequired(qint64 bufferSize, double sampleRate, const QString &sharedMemoryKey, int maxChannelCount) {
        Q_Q(RemoteAudioDevice);
        QMutexLocker locker(&mutex);

        sharedMemory.setNativeKey(sharedMemoryKey);
        if (sharedMemory.attach()) {
            remoteIsOpened = true;
            sharedAudioData.resize(maxChannelCount);
            for (int i = 0; i < maxChannelCount; i++) {
                sharedAudioData[i] = reinterpret_cast<float *>(sharedMemory.data()) + bufferSize * i;
            }
            buffer = new AudioDataWrapper(sharedAudioData.data(), maxChannelCount, bufferSize);

            q->setAvailableBufferSizes({bufferSize});
            q->setPreferredBufferSize(bufferSize);
            q->setAvailableSampleRates({sampleRate});
            q->setPreferredSampleRate(sampleRate);

            emit q->remoteOpened(bufferSize, sampleRate, maxChannelCount);
        } else {
            qWarning() << "RemoteAudioDevice: Cannot attach shared memory" << sharedMemoryKey;
        }
    }

    void RemoteAudioDevicePrivate::remoteCloseRequired() {
        Q_Q(RemoteAudioDevice);
        QMutexLocker locker(&mutex);

        delete buffer;
        buffer = nullptr;
        sharedAudioData.clear();
        sharedMemory.detach();

        q->setAvailableBufferSizes({});
        q->setPreferredBufferSize(0);
        q->setAvailableSampleRates({});
        q->setPreferredSampleRate(0);
        remoteIsOpened = false;
    }

    void RemoteAudioDevicePrivate::remotePrepareBuffer() {
        QMutexLocker locker(&mutex);
        Q_Q(RemoteAudioDevice);
        if (!q->isOpen()) {
            throw std::runtime_error("Remote audio device not opened.");
        }
        if (audioDeviceCallback) {
            audioDeviceCallback->workCallback(buffer);
        }
    }

    bool RemoteAudioDevice::open(qint64 bufferSize, double sampleRate) {
        Q_D(RemoteAudioDevice);
        QMutexLocker locker(&d->mutex);
        if (isOpen())
            close();
        if (!d->remoteIsOpened)
            return false;
        if (bufferSize != preferredBufferSize() || sampleRate != preferredSampleRate())
            return false;
        return AudioStreamBase::open(bufferSize, sampleRate);
    }

    void RemoteAudioDevice::close() {
        Q_D(RemoteAudioDevice);
        QMutexLocker locker(&d->mutex);
        AudioDevice::close();
    }

    bool RemoteAudioDevice::start(AudioDeviceCallback *audioDeviceCallback) {
        Q_D(RemoteAudioDevice);
        QMutexLocker locker(&d->mutex);
        d->audioDeviceCallback = audioDeviceCallback;
        audioDeviceCallback->deviceWillStartCallback(this);
        return AudioDevice::start(audioDeviceCallback);
    }
    void RemoteAudioDevice::stop() {
        Q_D(RemoteAudioDevice);
        QMutexLocker locker(&d->mutex);
        if (d->audioDeviceCallback) {
            d->audioDeviceCallback->deviceStoppedCallback();
            d->audioDeviceCallback = nullptr;
        }
        AudioDevice::stop();
    }

    void RemoteAudioDevice::lock() {
        Q_D(RemoteAudioDevice);
        d->mutex.lock();
    }
    void RemoteAudioDevice::unlock() {
        Q_D(RemoteAudioDevice);
        d->mutex.unlock();
    }

} // talcs