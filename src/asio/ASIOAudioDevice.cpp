/*******************************************************************************
 * Copyright (c) 2023 CrSjimo                                                  *
 * SPDX-License-Identifier: GPL-3.0-only                                       *
 *                                                                             *
 * This file is part of TALCS.                                                 *
 *                                                                             *
 * TALCS is free software: you can redistribute it and/or modify it under the  *
 * terms of the GNU General Public License as published by the Free Software   *
 * Foundation, version 3 of the License.                                       *
 *                                                                             *
 * TALCS is distributed in the hope that it will be useful, but WITHOUT ANY    *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS   *
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more       *
 * details.                                                                    *
 *                                                                             *
 * You should have received a copy of the GNU General Public License along     *
 * with TALCS. If not, see <https://www.gnu.org/licenses/>.                    *
 ******************************************************************************/

#include "ASIOAudioDevice_p.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstring>
#include <limits>
#include <utility>

#include <QDebug>
#include <QLoggingCategory>
#include <QMetaObject>
#include <QThread>
#include <QTimer>

#include <TalcsCore/AudioSource.h>

#include <TalcsAsio/private/ASIOAudioDriver_p.h>
#include <TalcsAsio/private/ASIOSampleConverter_p.h>

namespace talcs {

    Q_LOGGING_CATEGORY(lcASIOAudioDevice, "talcs.asio.device")

    static constexpr int DEVICE_LIST_SIZE = 16;
    static std::atomic<ASIOAudioDevicePrivate *> m_devices[DEVICE_LIST_SIZE]{};

    // Native driver dialogs can dispatch queued events before the driver call returns.
    class ASIODriverCallGuard {
        Q_DISABLE_COPY(ASIODriverCallGuard)
    public:
        explicit ASIODriverCallGuard(ASIOAudioDevicePrivate *device) : m_device(device) {
            m_device->driverCallDepth++;
        }

        ~ASIODriverCallGuard() {
            if (--m_device->driverCallDepth == 0 && m_device->notificationsEnabled.load() && m_device->pendingNotifications.load()) {
                m_device->notificationTimer->start();
            }
        }

    private:
        ASIOAudioDevicePrivate *m_device;
    };

    template<int deviceIndex>
    struct CallbackFunctions {
        static void sampleRateDidChange(ASIOSampleRate sampleRate) {
            if (auto d = m_devices[deviceIndex].load()) {
                d->notifiedSampleRate.store(sampleRate);
                d->requestNotification(ASIOAudioDevicePrivate::SampleRateChanged);
            }
        }

        static long asioMessage(long selector, long value, void *, double *) {
            auto d = m_devices[deviceIndex].load();
            if (!d) {
                return 0;
            }
            switch (selector) {
                case kAsioSelectorSupported:
                    switch (value) {
                        case kAsioEngineVersion:
                        case kAsioResetRequest:
                        case kAsioBufferSizeChange:
                        case kAsioResyncRequest:
                        case kAsioLatenciesChanged:
                        case kAsioSupportsTimeInfo:
                        case kAsioSupportsTimeCode:
                        case kAsioSupportsInputMonitor:
                            return 1;
                        default:
                            return 0;
                    }
                case kAsioEngineVersion:
                    return 2;
                case kAsioResetRequest:
                    d->requestNotification(ASIOAudioDevicePrivate::ResetRequested);
                    return 1;
                case kAsioBufferSizeChange:
                    if (value <= 0) {
                        return 0;
                    }
                    d->notifiedBufferSize.store(value);
                    d->requestNotification(ASIOAudioDevicePrivate::BufferSizeChanged);
                    return 1;
                case kAsioResyncRequest:
                    d->requestNotification(ASIOAudioDevicePrivate::ResyncRequested);
                    return 1;
                case kAsioLatenciesChanged:
                    d->requestNotification(ASIOAudioDevicePrivate::LatenciesChanged);
                    return 1;
                case kAsioSupportsTimeInfo:
                    return 1;
                default:
                    return 0;
            }
        }

        static ASIOTime *bufferSwitchTimeInfo(ASIOTime *timeInfo, long index, ASIOBool) {
            if (auto d = m_devices[deviceIndex].load()) {
                if (timeInfo && (timeInfo->timeInfo.flags & kSampleRateChanged) && (timeInfo->timeInfo.flags & kSampleRateValid)
                    && !qFuzzyCompare(timeInfo->timeInfo.sampleRate, d->streamingSampleRate.load())) {
                    sampleRateDidChange(timeInfo->timeInfo.sampleRate);
                }
                d->render(index);
            }
            return nullptr;
        }

        static void bufferSwitch(long index, ASIOBool) {
            if (auto d = m_devices[deviceIndex].load()) {
                d->render(index);
            }
        }
    };

    template<int maxDeviceIndex>
    static ASIOCallbacks getASIOCallbacks(int deviceIndex) {
        if (deviceIndex == maxDeviceIndex) {
            return {
                &CallbackFunctions<maxDeviceIndex>::bufferSwitch,
                &CallbackFunctions<maxDeviceIndex>::sampleRateDidChange,
                &CallbackFunctions<maxDeviceIndex>::asioMessage,
                &CallbackFunctions<maxDeviceIndex>::bufferSwitchTimeInfo,
            };
        }
        return getASIOCallbacks<maxDeviceIndex - 1>(deviceIndex);
    }

    template<>
    ASIOCallbacks getASIOCallbacks<-1>(int) {
        return {};
    }

    void ASIOAudioDevicePrivate::setIASIOError(const char *operation, long result) {
        Q_Q(ASIOAudioDevice);
        std::memset(errorMessageBuffer, 0, sizeof(errorMessageBuffer));
        if (iasio) {
            iasio->getErrorMessage(errorMessageBuffer);
        }
        errorMessageBuffer[sizeof(errorMessageBuffer) - 1] = '\0';
        auto message = QString::fromLocal8Bit(errorMessageBuffer).trimmed();
        if (message.isEmpty()) {
            message = ASIOAudioDevice::tr("The ASIO operation failed (%1).").arg(result);
        }
        q->setErrorString(message);
        qCWarning(lcASIOAudioDevice) << "ASIO device operation failed." << "operation:" << operation << "device:" << q->name()
                                   << "result:" << result << "error:" << message;
    }

    bool ASIOAudioDevicePrivate::initializeInterface() {
        Q_Q(ASIOAudioDevice);
        Q_ASSERT(!buffersCreated);
        releaseInterface();
        const auto result = CoCreateInstance(clsid, nullptr, CLSCTX_INPROC_SERVER, clsid, reinterpret_cast<void **>(&iasio));
        if (FAILED(result)) {
            q->setErrorString(asioSystemErrorString(result));
            return false;
        }
        if (!iasio->init(systemHandle)) {
            setIASIOError("IASIO::init", ASIOFalse);
            releaseInterface();
            return false;
        }
        if (!updateCapabilities()) {
            releaseInterface();
            return false;
        }
        postOutput = iasio->outputReady() == ASE_OK;
        q->setIsInitialized(true);
        q->setErrorString({});
        return true;
    }

    void ASIOAudioDevicePrivate::releaseInterface() {
        Q_Q(ASIOAudioDevice);
        q->setIsInitialized(false);
        if (iasio) {
            iasio->Release();
            iasio = nullptr;
        }
        driverRunning = false;
    }

    bool ASIOAudioDevicePrivate::updateCapabilities() {
        Q_Q(ASIOAudioDevice);
        long inputs = 0;
        long outputs = 0;
        if (const auto result = iasio->getChannels(&inputs, &outputs); result != ASE_OK) {
            setIASIOError("IASIO::getChannels", result);
            return false;
        }
        if (outputs <= 0) {
            q->setErrorString(ASIOAudioDevice::tr("The ASIO device has no output channels."));
            return false;
        }
        q->setChannelCount(static_cast<int>(outputs));

        long minimum = 0;
        long maximum = 0;
        long preferred = 0;
        long granularity = 0;
        if (const auto result = iasio->getBufferSize(&minimum, &maximum, &preferred, &granularity); result != ASE_OK) {
            setIASIOError("IASIO::getBufferSize", result);
            return false;
        }
        if (minimum <= 0 || maximum < minimum || preferred < minimum || preferred > maximum || granularity < -1) {
            q->setErrorString(ASIOAudioDevice::tr("The ASIO device reported invalid buffer sizes."));
            return false;
        }
        QList<qint64> sizes;
        if (granularity == -1) {
            for (qint64 size = minimum; size <= maximum; size *= 2) {
                sizes.append(size);
            }
        } else if (granularity > 0) {
            for (qint64 size = minimum; size <= maximum; size += granularity) {
                sizes.append(size);
            }
        }
        if (!sizes.contains(preferred)) {
            sizes.append(preferred);
            std::sort(sizes.begin(), sizes.end());
        }
        q->setAvailableBufferSizes(sizes);
        q->setPreferredBufferSize(preferred);

        double currentRate = 0;
        const auto result = iasio->getSampleRate(&currentRate);
        if (result != ASE_OK && result != ASE_NoClock) {
            setIASIOError("IASIO::getSampleRate", result);
            return false;
        }
        clockLost = result == ASE_NoClock || !std::isfinite(currentRate) || currentRate <= 0;
        static const QList<double> rates = {8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000,
            64000, 88200, 96000, 128000, 176400, 192000, 256000, 352800, 384000};
        QList<double> supportedRates;
        for (const auto rate : rates) {
            if (iasio->canSampleRate(rate) == ASE_OK) {
                supportedRates.append(rate);
            }
        }
        if (!clockLost && !supportedRates.contains(currentRate)) {
            supportedRates.append(currentRate);
            std::sort(supportedRates.begin(), supportedRates.end());
        }
        q->setAvailableSampleRates(supportedRates);
        q->setPreferredSampleRate(!clockLost ? currentRate : supportedRates.value(0));
        return true;
    }

    void ASIOAudioDevicePrivate::updateLatencies() {
        if (iasio && iasio->getLatencies(&inputLatency, &outputLatency) == ASE_OK) {
            qCDebug(lcASIOAudioDevice) << "ASIO device latencies refreshed." << "input samples:" << inputLatency << "output samples:" << outputLatency;
        }
    }

    void ASIOAudioDevicePrivate::requestNotification(unsigned int notification) {
        const auto generation = notificationGeneration.load();
        if (!notificationsEnabled.load()) {
            return;
        }
        Q_Q(ASIOAudioDevice);
        QMetaObject::invokeMethod(q, [this, generation, notification] {
            if (generation != notificationGeneration.load() || !notificationsEnabled.load()) {
                return;
            }
            pendingNotifications.fetch_or(notification);
            if (!driverCallDepth && !notificationTimer->isActive()) {
                notificationTimer->start();
            }
        }, Qt::QueuedConnection);
    }

    void ASIOAudioDevicePrivate::handleNotifications() {
        Q_Q(ASIOAudioDevice);
        if (driverCallDepth) {
            return;
        }
        ASIODriverCallGuard callGuard(this);
        const auto notifications = pendingNotifications.exchange(0);
        if (!notificationsEnabled.load() || !notifications) {
            return;
        }
        if (notifications & LatenciesChanged) {
            updateLatencies();
        }
        if (!(notifications & (ResetRequested | ResyncRequested | SampleRateChanged | BufferSizeChanged)) || !q->isOpen()) {
            return;
        }
        const auto oldBufferSize = q->bufferSize();
        const auto oldSampleRate = q->sampleRate();
        const auto newSampleRate = notifiedSampleRate.load();
        const auto newBufferSize = notifiedBufferSize.load();
        qCInfo(lcASIOAudioDevice) << "ASIO device recovery started." << "device:" << q->name() << "notifications:" << notifications;
        q->close();
        const bool initialized = ((notifications & ResetRequested) || !iasio) ? initializeInterface() : updateCapabilities();
        if (!initialized) {
            Q_EMIT q->closed();
            return;
        }
        recoveryBufferSize = (notifications & ResetRequested) ? q->preferredBufferSize() : oldBufferSize;
        recoverySampleRate = (notifications & ResetRequested) ? q->preferredSampleRate() : oldSampleRate;
        if ((notifications & BufferSizeChanged) && newBufferSize > 0) {
            recoveryBufferSize = newBufferSize;
        }
        if (notifications & SampleRateChanged) {
            if (std::isfinite(newSampleRate) && newSampleRate > 0) {
                recoverySampleRate = newSampleRate;
            } else {
                clockLost = true;
            }
        }
        if (clockLost) {
            recoverySampleRate = oldSampleRate;
            clockRecoveryTimer->start();
        }
        Q_EMIT q->closed();
    }

    void ASIOAudioDevicePrivate::checkClockRecovery() {
        Q_Q(ASIOAudioDevice);
        if (driverCallDepth) {
            return;
        }
        ASIODriverCallGuard callGuard(this);
        double rate = 0;
        if (!iasio || iasio->getSampleRate(&rate) != ASE_OK || !std::isfinite(rate) || rate <= 0) {
            return;
        }
        if (!updateCapabilities()) {
            return;
        }
        recoverySampleRate = rate;
        clockLost = false;
        clockRecoveryTimer->stop();
        Q_EMIT q->closed();
    }

    void ASIOAudioDevicePrivate::render(long index) {
        if (index != 0 && index != 1) {
            return;
        }
        QMutexLocker locker(&mutex);
        if (audioDeviceCallback) {
            audioBuffer.clear();
            audioDeviceCallback->workCallback(&audioBuffer);
        }
        for (int channel = 0; channel < bufferInfoList.size(); channel++) {
            auto dest = bufferInfoList[channel].buffers[index];
            if (audioDeviceCallback) {
                ASIOSampleConverter::convert(dest, audioBuffer.constData(channel), preparedBufferSize, channelInfoList[channel].type);
            } else {
                std::memset(dest, 0, static_cast<size_t>(preparedBufferSize) * ASIOSampleConverter::sampleSize(channelInfoList[channel].type));
            }
        }
        if (postOutput) {
            iasio->outputReady();
        }
    }

    ASIOAudioDevice::ASIOAudioDevice(const QString &name, const CLSID &clsid, HWND systemHandle, ASIOAudioDriver *driver)
        : AudioDevice(*new ASIOAudioDevicePrivate, driver) {
        Q_D(ASIOAudioDevice);
        setName(name);
        setDriver(driver);
        d->clsid = clsid;
        d->systemHandle = systemHandle;
        d->notificationTimer = new QTimer(this);
        d->notificationTimer->setSingleShot(true);
        d->notificationTimer->setInterval(0);
        connect(d->notificationTimer, &QTimer::timeout, this, [d] { d->handleNotifications(); });
        d->clockRecoveryTimer = new QTimer(this);
        d->clockRecoveryTimer->setInterval(500);
        connect(d->clockRecoveryTimer, &QTimer::timeout, this, [d] { d->checkClockRecovery(); });
        for (int index = 0; index < DEVICE_LIST_SIZE; index++) {
            ASIOAudioDevicePrivate *empty = nullptr;
            if (m_devices[index].compare_exchange_strong(empty, d)) {
                d->deviceIndex = index;
                break;
            }
        }
        if (d->deviceIndex == -1) {
            setErrorString(tr("The ASIO audio device instance limit has been reached."));
            return;
        }
        d->callbacks = getASIOCallbacks<DEVICE_LIST_SIZE - 1>(d->deviceIndex);
        if (d->initializeInterface()) {
            qCInfo(lcASIOAudioDevice) << "ASIO device initialization completed." << "device:" << name << "channels:" << channelCount();
        }
    }

    ASIOAudioDevice::~ASIOAudioDevice() {
        Q_D(ASIOAudioDevice);
        shutdown();
        if (d->deviceIndex != -1) {
            m_devices[d->deviceIndex].store(nullptr);
        }
    }

    void ASIOAudioDevice::shutdown() {
        Q_D(ASIOAudioDevice);
        ASIOAudioDevice::close();
        d->releaseInterface();
    }

    bool ASIOAudioDevice::open(qint64 bufferSize, double sampleRate) {
        Q_D(ASIOAudioDevice);
        ASIODriverCallGuard callGuard(d);
        Q_ASSERT(QThread::currentThread() == thread());
        if (!isInitialized()) {
            setErrorString(tr("The ASIO audio device is not initialized."));
            return false;
        }
        const auto requestedBufferSize = d->recoveryBufferSize > 0 ? d->recoveryBufferSize : bufferSize;
        const auto requestedSampleRate = d->recoverySampleRate > 0 ? d->recoverySampleRate : sampleRate;
        if (requestedBufferSize <= 0 || requestedBufferSize > std::numeric_limits<long>::max()
            || !std::isfinite(requestedSampleRate) || requestedSampleRate <= 0) {
            setErrorString(tr("The requested ASIO buffer size or sample rate is invalid."));
            return false;
        }
        const bool recoveringClock = d->clockRecoveryTimer->isActive();
        ASIOAudioDevice::close();
        if (!d->iasio && !d->initializeInterface()) {
            return false;
        }
        if (!d->updateCapabilities()) {
            return false;
        }
        double currentRate = 0;
        auto result = d->iasio->getSampleRate(&currentRate);
        if (result != ASE_OK && result != ASE_NoClock) {
            d->setIASIOError("IASIO::getSampleRate", result);
            return false;
        }
        if (result != ASE_OK || !qFuzzyCompare(currentRate, requestedSampleRate)) {
            result = d->iasio->setSampleRate(requestedSampleRate);
            if (result != ASE_OK) {
                d->setIASIOError("IASIO::setSampleRate", result);
                if (recoveringClock && d->clockLost) {
                    d->recoveryBufferSize = requestedBufferSize;
                    d->recoverySampleRate = requestedSampleRate;
                    d->clockRecoveryTimer->start();
                }
                return false;
            }
        }
        if (!d->updateCapabilities()) {
            return false;
        }
        if (d->clockLost || !qFuzzyCompare(preferredSampleRate(), requestedSampleRate)) {
            setErrorString(tr("The ASIO device did not apply the requested sample rate."));
            if (recoveringClock && d->clockLost) {
                d->recoveryBufferSize = requestedBufferSize;
                d->recoverySampleRate = requestedSampleRate;
                d->clockRecoveryTimer->start();
            }
            return false;
        }
        if (!availableBufferSizes().contains(requestedBufferSize) || activeChannelCount() <= 0) {
            setErrorString(tr("The requested ASIO buffer size or output channel count is not supported."));
            return false;
        }

        for (int channel = 0; channel < activeChannelCount(); channel++) {
            ASIOChannelInfo channelInfo{};
            channelInfo.channel = channel;
            channelInfo.isInput = ASIOFalse;
            result = d->iasio->getChannelInfo(&channelInfo);
            if (result != ASE_OK) {
                d->setIASIOError("IASIO::getChannelInfo", result);
                d->bufferInfoList.clear();
                d->channelInfoList.clear();
                return false;
            }
            if (!ASIOSampleConverter::sampleSize(channelInfo.type)) {
                setErrorString(tr("The ASIO device uses an unsupported sample format (%1).").arg(channelInfo.type));
                d->bufferInfoList.clear();
                d->channelInfoList.clear();
                return false;
            }
            d->bufferInfoList.append({ASIOFalse, channel, {nullptr, nullptr}});
            d->channelInfoList.append(channelInfo);
        }
        d->notificationsEnabled.store(true);
        result = d->iasio->createBuffers(d->bufferInfoList.data(), activeChannelCount(), static_cast<long>(requestedBufferSize), &d->callbacks);
        if (result != ASE_OK) {
            d->setIASIOError("IASIO::createBuffers", result);
            ASIOAudioDevice::close();
            return false;
        }
        d->buffersCreated = true;
        d->preparedBufferSize = requestedBufferSize;
        d->streamingSampleRate.store(requestedSampleRate);
        for (int channel = 0; channel < d->bufferInfoList.size(); channel++) {
            auto &channelInfo = d->channelInfoList[channel];
            result = d->iasio->getChannelInfo(&channelInfo);
            if (result != ASE_OK) {
                d->setIASIOError("IASIO::getChannelInfo", result);
                ASIOAudioDevice::close();
                return false;
            }
            if (!ASIOSampleConverter::sampleSize(channelInfo.type)) {
                setErrorString(tr("The ASIO device uses an unsupported sample format (%1).").arg(channelInfo.type));
                ASIOAudioDevice::close();
                return false;
            }
            for (auto buffer : d->bufferInfoList[channel].buffers) {
                if (!buffer) {
                    setErrorString(tr("The ASIO device returned an invalid audio buffer."));
                    ASIOAudioDevice::close();
                    return false;
                }
                std::memset(buffer, 0, static_cast<size_t>(requestedBufferSize) * ASIOSampleConverter::sampleSize(d->channelInfoList[channel].type));
            }
        }
        d->updateLatencies();
        setErrorString({});
        qCInfo(lcASIOAudioDevice) << "ASIO device opened." << "device:" << name() << "buffer size:" << requestedBufferSize << "sample rate:" << requestedSampleRate;
        return AudioDevice::open(requestedBufferSize, requestedSampleRate);
    }

    void ASIOAudioDevice::close() {
        Q_D(ASIOAudioDevice);
        ASIODriverCallGuard callGuard(d);
        d->notificationsEnabled.store(false);
        d->notificationGeneration.fetch_add(1);
        d->pendingNotifications.store(0);
        d->notificationTimer->stop();
        d->clockRecoveryTimer->stop();
        ASIOAudioDevice::stop();
        if (d->buffersCreated && d->iasio) {
            if (const auto result = d->iasio->disposeBuffers(); result != ASE_OK) {
                d->setIASIOError("IASIO::disposeBuffers", result);
                d->releaseInterface();
            }
        }
        d->driverRunning = false;
        d->buffersCreated = false;
        d->preparedBufferSize = 0;
        d->bufferInfoList.clear();
        d->channelInfoList.clear();
        d->recoveryBufferSize = 0;
        d->recoverySampleRate = 0;
        d->streamingSampleRate.store(0);
        AudioDevice::close();
    }

    bool ASIOAudioDevice::start(AudioDeviceCallback *audioDeviceCallback) {
        Q_D(ASIOAudioDevice);
        ASIODriverCallGuard callGuard(d);
        Q_ASSERT(QThread::currentThread() == thread());
        if (!isInitialized() || !isOpen() || !d->buffersCreated || !audioDeviceCallback) {
            setErrorString(tr("ASIO playback requires an open device and an audio callback."));
            return false;
        }
        if (isStarted() && d->audioDeviceCallback == audioDeviceCallback) {
            return true;
        }
        ASIOAudioDevice::stop();
        if (d->driverRunning) {
            return false;
        }
        d->audioBuffer.resize(activeChannelCount(), bufferSize());
        d->audioBuffer.clear();
        if (!audioDeviceCallback->deviceWillStartCallback(this)) {
            d->audioBuffer.resize(0, 0);
            setErrorString(tr("The audio callback could not prepare ASIO playback."));
            return false;
        }
        {
            QMutexLocker locker(&d->mutex);
            d->audioDeviceCallback = audioDeviceCallback;
        }
        // Driver calls can wait for callbacks on another thread.
        d->driverRunning = true;
        const auto result = d->iasio->start();
        if (result != ASE_OK) {
            d->setIASIOError("IASIO::start", result);
            const auto startError = errorString();
            ASIOAudioDevice::stop();
            setErrorString(startError);
            return false;
        }
        setErrorString({});
        return AudioDevice::start(audioDeviceCallback);
    }

    void ASIOAudioDevice::stop() {
        Q_D(ASIOAudioDevice);
        ASIODriverCallGuard callGuard(d);
        AudioDeviceCallback *callback = nullptr;
        {
            QMutexLocker locker(&d->mutex);
            callback = std::exchange(d->audioDeviceCallback, nullptr);
        }
        if (d->iasio && d->driverRunning) {
            if (const auto result = d->iasio->stop(); result != ASE_OK) {
                d->setIASIOError("IASIO::stop", result);
            } else {
                d->driverRunning = false;
            }
        }
        AudioDevice::stop();
        d->audioBuffer.resize(0, 0);
        if (callback) {
            callback->deviceStoppedCallback();
        }
    }

    void ASIOAudioDevice::lock() {
        Q_D(ASIOAudioDevice);
        d->mutex.lock();
    }

    void ASIOAudioDevice::unlock() {
        Q_D(ASIOAudioDevice);
        d->mutex.unlock();
    }

    bool ASIOAudioDevice::openControlPanel() {
        Q_D(ASIOAudioDevice);
        ASIODriverCallGuard callGuard(d);
        Q_ASSERT(QThread::currentThread() == thread());
        if (!isInitialized() || !d->iasio) {
            setErrorString(tr("The ASIO audio device is not initialized."));
            return false;
        }
        const auto result = d->iasio->controlPanel();
        if (result != ASE_OK) {
            d->setIASIOError("IASIO::controlPanel", result);
            return false;
        }
        return true;
    }
}
