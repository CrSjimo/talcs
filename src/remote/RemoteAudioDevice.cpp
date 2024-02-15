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

#include "RemoteAudioDevice.h"
#include "RemoteAudioDevice_p.h"

#include <chrono>

#include "RemoteSocket.h"

#include <TalcsCore/AudioSource.h>

#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/sync/named_condition.hpp>

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

    void RemoteAudioDevicePrivate::remoteOpenRequired(qint64 bufferSize, double sampleRate, const QString &ipcKey, int maxChannelCount) {
        Q_Q(RemoteAudioDevice);
        using namespace boost::interprocess;
        QMutexLocker locker(&mutex);

        sharedMemory.setNativeKey(ipcKey);
        if (sharedMemory.attach()) {
            remoteIsOpened = true;
            sharedAudioData.resize(maxChannelCount);
            auto *ptr = reinterpret_cast<char *>(sharedMemory.data());
            for (int i = 0; i < maxChannelCount; i++) {
                sharedAudioData[i] = reinterpret_cast<float *>(ptr);
                ptr += bufferSize * sizeof(float);
            }
            bufferPrepareStatus = reinterpret_cast<char *>(ptr);
            ptr += sizeof(bool);
            processInfo = reinterpret_cast<RemoteAudioDevice::ProcessInfo *>(ptr);
            buffer.reset(new AudioDataWrapper(sharedAudioData.data(), maxChannelCount, bufferSize));

            q->setAvailableBufferSizes({bufferSize});
            q->setPreferredBufferSize(bufferSize);
            q->setAvailableSampleRates({sampleRate});
            q->setPreferredSampleRate(sampleRate);
        } else {
            qWarning() << "RemoteAudioDevice: Cannot attach shared memory" << ipcKey;
        }
        prepareBufferMutex.reset(new named_mutex(open_only, ipcKey.toLatin1()));
        prepareBufferCondition.reset(new named_condition(open_only, (ipcKey + "cv").toLatin1()));
        prepareBufferProducerThread.reset(QThread::create([this] { remotePrepareBufferProducer(); }));
        prepareBufferProducerThread->start(QThread::HighestPriority);
        emit q->remoteOpened(bufferSize, sampleRate, maxChannelCount);
    }

    void RemoteAudioDevicePrivate::remoteCloseRequired() {
        Q_Q(RemoteAudioDevice);
        QMutexLocker locker(&mutex);

        if (prepareBufferProducerThread) {
            prepareBufferProducerThread->requestInterruption();
            prepareBufferProducerThread->quit();
            prepareBufferProducerThread->wait();
        }
        prepareBufferProducerThread.reset();
        prepareBufferCondition.reset();
        prepareBufferMutex.reset();
        buffer.reset();
        sharedAudioData.clear();
        bufferPrepareStatus = nullptr;
        processInfo = nullptr;
        sharedMemory.detach();

        q->setAvailableBufferSizes({});
        q->setPreferredBufferSize(0);
        q->setAvailableSampleRates({});
        q->setPreferredSampleRate(0);
        remoteIsOpened = false;
        q->close();
        emit q->closed();
    }

    void RemoteAudioDevicePrivate::remotePrepareBufferProducer() {
        using namespace std::chrono_literals;
        using namespace boost::interprocess;
        for (;;) {
            scoped_lock<named_mutex> lock(*prepareBufferMutex, std::chrono::system_clock::now() + 1000ms);
            if (prepareBufferProducerThread->isInterruptionRequested())
                break;
            if (*bufferPrepareStatus == GoingToClose)
                break;
            if (!lock.owns())
                continue;
            bool success = prepareBufferCondition->wait_for(lock, 1000ms, [=] { return *bufferPrepareStatus != Prepared; });
            if (success) {
                remotePrepareBuffer();
                *bufferPrepareStatus = Prepared;
                prepareBufferCondition->notify_one();
            }
        }
    }

    void RemoteAudioDevicePrivate::remotePrepareBuffer() {
        QMutexLocker locker(&mutex);
        Q_Q(RemoteAudioDevice);
        if (!q->isOpen()) {
            return;
        }
        if (processInfo->containsInfo) {
            for (auto processInfoCallback: processInfoCallbackList) {
                processInfoCallback->onThisBlockProcessInfo(*processInfo);
            }
        }
        if (audioDeviceCallback)
            audioDeviceCallback->workCallback(buffer.get());
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