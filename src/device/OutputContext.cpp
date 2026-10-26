/******************************************************************************
 * Copyright (c) 2024 CrSjimo                                                 *
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

#include "OutputContext.h"
#include "OutputContext_p.h"

#include <QDebug>
#include <QLoggingCategory>
#include <QSignalBlocker>

#include <TalcsCore/MixerAudioSource.h>

#include <TalcsDevice/AudioDriverManager.h>
#include <TalcsDevice/AudioDevice.h>
#include <TalcsDevice/AudioDriver.h>
#include <TalcsDevice/AudioSourcePlayback.h>

namespace talcs {

    Q_LOGGING_CATEGORY(lcOutputContext, "talcs.device.outputContext")

    /**
     * @class OutputContext
     * @brief The derived AbstractOutputContext based on the audio driver manager created by
     * AudioDriverManager::createBuiltInDriverManager()
     */

    /**
     * Creates an output context with all built-in drivers.
     */
    OutputContext::OutputContext(QObject *parent) : AbstractOutputContext(parent), d_ptr(new OutputContextPrivate) {
        Q_D(OutputContext);
        d->q_ptr = this;
        d->driverManager.reset(AudioDriverManager::createBuiltInDriverManager());
    }

    /**
     * Destructor.
     */
    OutputContext::~OutputContext() = default;

    /**
     * Initializes the OutputContext.
     *
     * For customized purpose, you can also call setDriver() and setDevice() to manually initialize,
     * or call enumerateDevice() to manually enumerate audio drivers and devices.
     *
     * @param driverNameHint hint of the name of the driver to use
     * @param deviceNameHint hint of the name of the device to use
     * @return true if successful
     */
    bool OutputContext::initialize(const QString &driverNameHint, const QString &deviceNameHint) {
        Q_D(OutputContext);
        qCInfo(lcOutputContext) << "Audio output initialization started." << "driver hint:" << driverNameHint << "device hint:" << deviceNameHint;
        do {
            if (driverNameHint.isEmpty())
                break;
            d->driver = d->driverManager->driver(driverNameHint);
            if (!d->driver || !d->driver->initialize()) {
                qCWarning(lcOutputContext) << "Requested audio driver initialization failed." << "driver:" << driverNameHint
                                          << "error:" << (d->driver ? d->driver->errorString() : QStringLiteral("The audio driver is not registered."));
                d->driver = nullptr;
                break;
            }
            if (deviceNameHint.isEmpty())
                break;
            std::unique_ptr<talcs::AudioDevice> dev(d->driver->createDevice(deviceNameHint));
            if (!dev || !dev->isInitialized()) {
                qCWarning(lcOutputContext) << "Requested audio device initialization failed."
                                          << "driver:" << d->driver->name() << "device:" << deviceNameHint
                                          << "error:" << (dev ? dev->errorString() : d->driver->errorString());
                break;
            }
            if (!d->openDeviceWithOption(dev.get(), DO_DefaultOption))
                break;
            d->device = std::move(dev);
            AbstractOutputContext::setDevice(d->device.get());
            d->postSetDevice();
            connect(d->driver, &AudioDriver::deviceChanged, this, &OutputContext::handleDriverDeviceChanged, Qt::UniqueConnection);
            return true;
        } while (false);
        qCInfo(lcOutputContext) << "Audio output initialization is falling back to driver and device enumeration.";
        return enumerateDevices();
    }

    /**
     * Gets the audio driver manager.
     */
    AudioDriverManager *OutputContext::driverManager() const {
        Q_D(const OutputContext);
        return d->driverManager.get();
    }

    /**
     * Gets the audio driver.
     */
    AudioDriver *OutputContext::driver() const {
        Q_D(const OutputContext);
        return d->driver;
    }

    /**
     * @enum OutputContext::DeviceOption
     * The options of audio device initialization
     *
     * @var OutputContext::DO_DefaultOption
     * The adopted buffer size and sample rate will be used first to initialize the audio device. If
     * it fails, then the preferred buffer size and sample rate will be used and the adopted buffer
     * size and sample rate will be changed.
     *
     * @var OutputContext::DO_UsePreferredSpec
     * Uses the preferred buffer size and sample rate to initialize the audio device.
     *
     * @var OutputContext::DO_DoNotChangeAdoptedSpec
     * Uses the adopted buffer size and sample rate to initialize the audio device.
     */

    /**
     * Sets the audio driver.
     *
     * If driverName is identical to the name of current driver, and current driver and device are
     * ready, this function will do nothing.
     *
     * Unlike setDevice(), if this function fails, current device and driver will become nullptr.
     *
     * @param driverName the name of the driver to be used
     * @param option the option of audio device initialization
     * @return true if successful
     */
    bool OutputContext::setDriver(const QString &driverName, OutputContext::DeviceOption option) {
        Q_D(OutputContext);
        qCInfo(lcOutputContext) << "Audio driver selection started." << "driver:" << driverName << "device option:" << option;
        if (d->driver && driverName == d->driver->name() && d->driver->isInitialized() && d->device && d->device->isOpen()) {
            qCDebug(lcOutputContext) << "The selected audio driver and device are already open." << "driver:" << driverName << "device:" << d->device->name();
            return true;
        }
        {
            // Clear the borrowed pointer before destruction and notify after driver selection completes.
            const QSignalBlocker blocker(this);
            AbstractOutputContext::setDevice(nullptr);
        }
        d->device.reset();
        if (d->driver) {
            disconnect(d->driver, nullptr, this, nullptr);
            d->driver->finalize();
        }
        d->driver = d->driverManager->driver(driverName);
        if (!d->driver || !d->driver->initialize()) {
            qCWarning(lcOutputContext) << "Audio driver selection failed." << "driver:" << driverName
                                      << "error:" << (d->driver ? d->driver->errorString() : QStringLiteral("The audio driver is not registered."));
            d->driver = nullptr;
            Q_EMIT deviceChanged();
            return false;
        }
        const auto succeeded = enumerateDevices(option);
        if (!succeeded) {
            Q_EMIT deviceChanged();
        }
        return succeeded;
    }

    /**
     * Sets the audio device.
     *
     * Unlike setDriver(), if this function fails, current device still remains unchanged.
     *
     * @param deviceName the name of the device to be used
     * @param option the option of audio device initialization
     * @return true if successful
     */
    bool OutputContext::setDevice(const QString &deviceName, OutputContext::DeviceOption option) {
        Q_D(OutputContext);
        qCInfo(lcOutputContext) << "Audio device selection started." << "device:" << deviceName << "device option:" << option;
        if (!d->driver || deviceName.isEmpty()) {
            qCWarning(lcOutputContext) << "Audio device selection requires an active driver and a nonempty device name." << "device:" << deviceName;
            return false;
        }
        std::unique_ptr<talcs::AudioDevice> dev(d->driver->createDevice(deviceName));
        if (!dev || !dev->isInitialized()) {
            qCWarning(lcOutputContext) << "Audio device initialization failed." << "driver:" << d->driver->name() << "device:" << deviceName
                                      << "error:" << (dev ? dev->errorString() : d->driver->errorString());
            return false;
        }
        if (!d->openDeviceWithOption(dev.get(), option))
            return false;
        d->device = std::move(dev);
        AbstractOutputContext::setDevice(d->device.get());
        d->postSetDevice();
        return true;
    }

    /**
     * Enumerate all audio drivers and devices to find a proper one to use.
     *
     * If an audio driver is set and able to be initialized, then it will be used. Otherwise, the first
     * successfully initialized driver will be used.
     *
     * If current driver has a default device then it will be used. Otherwise, the first successfully
     * initialized device will be used.
     *
     * @param option the option of audio device initialization
     * @return true if successful
     */
    bool OutputContext::enumerateDevices(OutputContext::DeviceOption option) {
        Q_D(OutputContext);
        qCInfo(lcOutputContext) << "Audio driver and device enumeration started." << "drivers:" << d->driverManager->drivers();
        for (int i = 0;; i++) {
            if (d->driver && d->driver->initialize()) {
                break;
            }
            if (d->driver) {
                qCWarning(lcOutputContext) << "Audio driver initialization failed during enumeration." << "driver:" << d->driver->name() << "error:" << d->driver->errorString();
            }
            if (i >= d->driverManager->drivers().size()) {
                qCCritical(lcOutputContext) << "Audio driver enumeration failed because no driver could be initialized.";
                d->driver = nullptr;
                return false;
            }
            d->driver = d->driverManager->driver(d->driverManager->drivers()[i]);
            if (d->driver && d->driver->initialize()) {
                break;
            }
            if (d->driver) {
                qCWarning(lcOutputContext) << "Audio driver initialization failed during enumeration." << "driver:" << d->driver->name() << "error:" << d->driver->errorString();
            }
        }
        auto deviceNames = d->driver->devices();
        deviceNames.removeAll(QString());
        const auto defaultDeviceName = d->driver->defaultDevice();
        qCInfo(lcOutputContext) << "Audio devices enumerated." << "driver:" << d->driver->name()
                               << "devices:" << deviceNames << "default device:" << defaultDeviceName;
        if (!defaultDeviceName.isEmpty()) {
            deviceNames.removeAll(defaultDeviceName);
            deviceNames.prepend(defaultDeviceName);
        }
        for (const auto &deviceName : deviceNames) {
            qCInfo(lcOutputContext) << "Audio device initialization started." << "driver:" << d->driver->name() << "device:" << deviceName;
            std::unique_ptr<talcs::AudioDevice> currentDevice(d->driver->createDevice(deviceName));
            if (!currentDevice || !currentDevice->isInitialized()) {
                qCWarning(lcOutputContext) << "Audio device initialization failed." << "driver:" << d->driver->name() << "device:" << deviceName
                                          << "error:" << (currentDevice ? currentDevice->errorString() : d->driver->errorString());
                continue;
            }
            if (!d->openDeviceWithOption(currentDevice.get(), option)) {
                continue;
            }
            d->device = std::move(currentDevice);
            AbstractOutputContext::setDevice(d->device.get());
            d->postSetDevice();
            connect(d->driver, &AudioDriver::deviceChanged, this, &OutputContext::handleDriverDeviceChanged, Qt::UniqueConnection);
            return true;
        }
        qCWarning(lcOutputContext) << "Audio device enumeration failed because no device could be opened." << "driver:" << d->driver->name();
        return false;
    }

    /**
     * Gets the adopted buffer size.
     */
    qint64 OutputContext::adoptedBufferSize() const {
        Q_D(const OutputContext);
        return d->adoptedBufferSize;
    }

    /**
     * Sets the adopted buffer size.
     *
     * This will reopen the device and the pre-mixer.
     *
     * @return true if successful
     */
    bool OutputContext::setAdoptedBufferSize(qint64 bufferSize) {
        Q_D(OutputContext);
        if (bufferSize == d->adoptedBufferSize)
            return true;
        d->adoptedBufferSize = bufferSize;
        qCInfo(lcOutputContext) << "Audio buffer size change started." << "buffer size:" << bufferSize;
        if (d->device && d->device->isOpen()) {
            if (!d->device->open(d->adoptedBufferSize, d->adoptedSampleRate)) {
                qCWarning(lcOutputContext) << "Audio device reopen failed after a buffer size change." << "device:" << d->device->name()
                                          << "buffer size:" << bufferSize << "sample rate:" << d->adoptedSampleRate << "error:" << d->device->errorString();
                return false;
            }
        }
        emit bufferSizeChanged(bufferSize);
        if (!controlMixer()->open(d->adoptedBufferSize, d->adoptedSampleRate)) {
            qCWarning(lcOutputContext) << "Audio mixer open failed after a buffer size change." << "buffer size:" << bufferSize << "sample rate:" << d->adoptedSampleRate;
        }
        if (d->device && d->device->isOpen() && !d->device->start(playback())) {
            qCWarning(lcOutputContext) << "Audio device restart failed after a buffer size change." << "device:" << d->device->name() << "error:" << d->device->errorString();
        }
        return true;
    }

    /**
     * Gets the adopted sample rate.
     */
    double OutputContext::adoptedSampleRate() const {
        Q_D(const OutputContext);
        return d->adoptedSampleRate;
    }

    /**
     * Sets the adopted sample rate.
     *
     * This will reopen the device and the pre-mixer.
     *
     * @return true if successful
     */
    bool OutputContext::setAdoptedSampleRate(double sampleRate) {
        Q_D(OutputContext);
        if (qFuzzyCompare(sampleRate, d->adoptedSampleRate))
            return true;
        d->adoptedSampleRate = sampleRate;
        qCInfo(lcOutputContext) << "Audio sample rate change started." << "sample rate:" << sampleRate;
        if (d->device && d->device->isOpen()) {
            if (!d->device->open(d->adoptedBufferSize, d->adoptedSampleRate)) {
                qCWarning(lcOutputContext) << "Audio device reopen failed after a sample rate change." << "device:" << d->device->name()
                                          << "buffer size:" << d->adoptedBufferSize << "sample rate:" << sampleRate << "error:" << d->device->errorString();
                return false;
            }
            if (!d->device->start(playback())) {
                qCWarning(lcOutputContext) << "Audio device restart failed after a sample rate change." << "device:" << d->device->name() << "error:" << d->device->errorString();
            }
        }
        emit sampleRateChanged(sampleRate);
        if (!controlMixer()->open(d->adoptedBufferSize, d->adoptedSampleRate)) {
            qCWarning(lcOutputContext) << "Audio mixer open failed after a sample rate change." << "buffer size:" << d->adoptedBufferSize << "sample rate:" << sampleRate;
        }
        return true;
    }

    /**
     * @enum OutputContext::HotPlugNotificationMode
     * The mode of hot-plug notification.
     *
     * @var OutputContext::Omni
     * Notifies on any device is added or removed.
     *
     * @var OutputContext::Current
     * Notifies on current device is removed.
     *
     * @var OutputContext::None
     * Does not notify.
     */

    /**
     * Gets the hot-plug notification mode.
     */
    OutputContext::HotPlugNotificationMode OutputContext::hotPlugNotificationMode() const {
        Q_D(const OutputContext);
        return d->hotPlugNotificationMode;
    }

    /**
     * Sets the hot-plug notification mode.
     */
    void OutputContext::setHotPlugNotificationMode(OutputContext::HotPlugNotificationMode mode) {
        Q_D(OutputContext);
        d->hotPlugNotificationMode = mode;
    }

    void OutputContext::handleDriverDeviceChanged() {
        Q_D(OutputContext);
        if (sender() == d->driver) {
            d->handleDeviceHotPlug();
        }
    }

    void OutputContext::handleDeviceClosed() {
        Q_D(OutputContext);
        if (!d->device || sender() != d->device.get() || !d->driver || !d->driver->isInitialized() || d->device->isOpen()) {
            return;
        }
        qCWarning(lcOutputContext) << "The current audio device closed unexpectedly." << "driver:" << d->driver->name()
                                  << "device:" << d->device->name() << "error:" << d->device->errorString();
        if (!d->driver->devices().contains(d->device->name())) {
            return;
        }
        if (d->openDeviceWithOption(d->device.get(), DO_DefaultOption)) {
            d->postSetDevice();
        }
    }

    /**
     * @fn OutputContext::deviceHotPlugged()
     * Emitted when hot-plug is detected.
     *
     * @see HotPlugNotificationMode
     */
    bool OutputContextPrivate::openDeviceWithOption(AudioDevice *device, OutputContext::DeviceOption option) const {
        auto savedBufferSize = adoptedBufferSize;
        auto savedSampleRate = adoptedSampleRate;
        switch (option) {
            case OutputContext::DO_DefaultOption:
                savedBufferSize = !savedBufferSize ? device->preferredBufferSize() : savedBufferSize;
                savedSampleRate = qFuzzyIsNull(savedSampleRate) ? device->preferredSampleRate() : savedSampleRate;
                break;
            case OutputContext::DO_UsePreferredSpec:
                savedBufferSize = device->preferredBufferSize();
                savedSampleRate = device->preferredSampleRate();
                break;
            case OutputContext::DO_DoNotChangeAdoptedSpec:
                break;
        }
        qCInfo(lcOutputContext) << "Audio device open started." << "driver:" << driver->name() << "device:" << device->name()
                               << "buffer size:" << savedBufferSize << "sample rate:" << savedSampleRate;
        if (!device->open(savedBufferSize, savedSampleRate)) {
            qCWarning(lcOutputContext) << "Audio device open failed. The preferred format will be retried."
                                      << "driver:" << driver->name() << "device:" << device->name()
                                      << "buffer size:" << savedBufferSize << "sample rate:" << savedSampleRate << "error:" << device->errorString();
            qCInfo(lcOutputContext) << "Audio device open is retrying with the preferred format." << "device:" << device->name()
                                   << "buffer size:" << device->preferredBufferSize() << "sample rate:" << device->preferredSampleRate();
            if (!device->open(device->preferredBufferSize(), device->preferredSampleRate())) {
                qCWarning(lcOutputContext) << "Audio device open failed with the preferred format." << "driver:" << driver->name()
                                          << "device:" << device->name() << "error:" << device->errorString();
                return false;
            }
        }
        qCInfo(lcOutputContext) << "Audio device open completed." << "driver:" << driver->name() << "device:" << device->name()
                               << "buffer size:" << device->bufferSize() << "sample rate:" << device->sampleRate();
        return true;
    }
    void OutputContextPrivate::handleDeviceHotPlug() {
        Q_Q(OutputContext);
        auto deviceList = driver->devices();
        qCInfo(lcOutputContext) << "Audio device hot plug detected." << "driver:" << driver->name() << "devices:" << deviceList
                               << "current device:" << (device ? device->name() : QString()) << "notification mode:" << hotPlugNotificationMode;
        if (device && deviceList.contains(device->name()) && !device->isOpen()) {
            if (!openDeviceWithOption(device.get(), OutputContext::DO_DefaultOption)) {
                qCWarning(lcOutputContext) << "Audio device recovery failed after hot plug." << "device:" << device->name() << "error:" << device->errorString();
            } else {
                postSetDevice();
            }
        }
        switch (hotPlugNotificationMode) {
            case OutputContext::Omni:
                emit q->deviceHotPlugged();
                break;
            case OutputContext::Current:
                if (device && !driver->devices().contains(device->name()))
                    emit q->deviceHotPlugged();
                break;
            case OutputContext::None:
                break;
        }
    }

    void OutputContextPrivate::postSetDevice() {
        Q_Q(OutputContext);
        QObject::connect(device.get(), &AudioDevice::closed, q, &OutputContext::handleDeviceClosed,
            static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::UniqueConnection));
        if (adoptedBufferSize != device->bufferSize()) {
            adoptedBufferSize = device->bufferSize();
            emit q->bufferSizeChanged(adoptedBufferSize);
        }
        if (!qFuzzyCompare(adoptedSampleRate, device->sampleRate())) {
            adoptedSampleRate = device->sampleRate();
            emit q->sampleRateChanged(adoptedSampleRate);
        }
        if (!q->controlMixer()->open(adoptedBufferSize, adoptedSampleRate)) {
            qCWarning(lcOutputContext) << "Audio mixer open failed." << "device:" << device->name()
                                      << "buffer size:" << adoptedBufferSize << "sample rate:" << adoptedSampleRate;
        }
        if (!device->start(q->playback())) {
            qCWarning(lcOutputContext) << "Audio device playback start failed." << "driver:" << driver->name()
                                      << "device:" << device->name() << "error:" << device->errorString();
        } else {
            qCInfo(lcOutputContext) << "Audio output playback started." << "driver:" << driver->name() << "device:" << device->name()
                                   << "buffer size:" << adoptedBufferSize << "sample rate:" << adoptedSampleRate << "channels:" << device->activeChannelCount();
        }
        emit q->deviceChanged();
    }
}
