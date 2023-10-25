#include "RemoteAudioDevice.h"
#include "RemoteAudioDevice_p.h"

#include "RemoteSocket.h"
#include <TalcsCore/AudioSource.h>

namespace talcs {

    /**
     * @struct RemoteAudioDevice::ProcessInfo
     * @brief The information of each process context
     *
     */

    /**
     * @var RemoteAudioDevice::ProcessInfo::containsInfo
     * When this value equals zero, the ProcessInfo struct is null.
     */

    /**
     * @enum RemoteAudioDevice::ProcessInfo::PlaybackStatus
     * Current playback status
     *
     * @var RemoteAudioDevice::ProcessInfo::NotPlaying
     * The remote audio context is not playing
     *
     * @var RemoteAudioDevice::ProcessInfo::Playing
     * The remote audio context is playing
     *
     * @var RemoteAudioDevice::ProcessInfo::RealtimePlaying
     * The remote audio context is playing in realtime
     */

    /**
     * @var RemoteAudioDevice::ProcessInfo::status
     * Current playback status
     */

    /**
     * @var RemoteAudioDevice::ProcessInfo::timeSignatureNumerator
     * The numerator of remote audio context. Equals zero when this field is not supported.
     */

    /**
     * @var RemoteAudioDevice::ProcessInfo::timeSignatureDenominator
     * The denominator of remote audio context. Equals zero when this field is not supported.
     */

    /**
     * @var RemoteAudioDevice::ProcessInfo::tempo
     * The tempo (bpm) of remote audio context. Equals zero when this field is not supported.
     */

    /**
     * @var RemoteAudioDevice::ProcessInfo::position
     * The position of remote audio context
     */

    /**
     * @class RemoteAudioDevice::ProcessInfoCallback
     * @brief The callback of ProcessInfo. Called before each audio block is processed.
     */

    /**
     * @fn void RemoteAudioDevice::ProcessInfoCallback::onThisBlockProcessInfo(const RemoteAudioDevice::ProcessInfo &processInfo)
     * The work function.
     */

    /**
     * @class RemoteAudioDevice
     * @brief The device that plays audio to a remote audio context.
     */

    /**
     * Constructor.
     * @param socket the remote socket where this device runs
     * @param name the name of this device
     */
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

    /**
     * Destructor.
     *
     * Note that if the device is opened, it will be closed now.
     */
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
            auto *sharedMemoryPtr = reinterpret_cast<char *>(sharedMemory.data());
            for (int i = 0; i < maxChannelCount; i++) {
                sharedAudioData[i] = reinterpret_cast<float *>(sharedMemoryPtr);
                sharedMemoryPtr += bufferSize * sizeof(float);
            }
            processInfo = reinterpret_cast<RemoteAudioDevice::ProcessInfo *>(sharedMemoryPtr);
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
        processInfo = nullptr;
        sharedMemory.detach();

        q->setAvailableBufferSizes({});
        q->setPreferredBufferSize(0);
        q->setAvailableSampleRates({});
        q->setPreferredSampleRate(0);
        remoteIsOpened = false;
        q->close();
    }

    void RemoteAudioDevicePrivate::remotePrepareBuffer() {
        QMutexLocker locker(&mutex);
        Q_Q(RemoteAudioDevice);
        if (!q->isOpen()) {
            throw std::runtime_error("Remote audio device not opened.");
        }
        if (processInfo->containsInfo) {
            for (auto processInfoCallback: processInfoCallbackList) {
                processInfoCallback->onThisBlockProcessInfo(*processInfo);
            }
        }
        if (audioDeviceCallback)
            audioDeviceCallback->workCallback(buffer);
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
        stop();
        AudioDevice::close();
    }

    /**
     * Adds a ProcessInfoCallback.
     */
    void RemoteAudioDevice::addProcessInfoCallback(RemoteAudioDevice::ProcessInfoCallback *callback) {
        Q_D(RemoteAudioDevice);
        d->processInfoCallbackList.append(callback);
    }

    /**
     * Removes a ProcessInfoCallback.
     */
    void RemoteAudioDevice::removeProcessInfoCallback(RemoteAudioDevice::ProcessInfoCallback *callback) {
        Q_D(RemoteAudioDevice);
        d->processInfoCallbackList.removeOne(callback);
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

    /**
     * @fn void RemoteAudioDevice::remoteOpened(qint64 bufferSize, double sampleRate, int maxChannelCount)
     * Emitted when the RemoteAudioSource prepares to play.
     */

}