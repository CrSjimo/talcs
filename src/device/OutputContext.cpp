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

#include <TalcsCore/MixerAudioSource.h>

#include <TalcsDevice/AudioDriverManager.h>
#include <TalcsDevice/AudioDevice.h>
#include <TalcsDevice/AudioDriver.h>
#include <TalcsDevice/AudioSourcePlayback.h>

namespace talcs {
    OutputContext::OutputContext(QObject *parent) : AbstractOutputContext(parent), d_ptr(new OutputContextPrivate) {
        Q_D(OutputContext);
        d->q_ptr = this;
        d->driverManager.reset(AudioDriverManager::createBuiltInDriverManager());
    }

    OutputContext::~OutputContext() = default;

    bool OutputContext::initialize(const QString &driverNameHint, const QString &deviceNameHint) {
        Q_D(OutputContext);
        do {
            if (driverNameHint.isEmpty())
                break;
            d->driver = d->driverManager->driver(driverNameHint);
            if (!d->driver || !d->driver->initialize()) {
                d->driver = nullptr;
                break;
            }
            if (deviceNameHint.isEmpty())
                break;
            std::unique_ptr<talcs::AudioDevice> dev(d->driver->createDevice(deviceNameHint));
            if (!dev || !dev->isInitialized())
                break;
            auto savedBufferSize = d->adoptedBufferSize == 0 ? dev->preferredBufferSize() : d->adoptedBufferSize;
            auto savedSampleRate = qFuzzyIsNull(d->adoptedSampleRate) ? dev->preferredSampleRate() : d->adoptedSampleRate;
            if (!dev->open(savedBufferSize, savedSampleRate) && !dev->open(dev->preferredBufferSize(), dev->preferredSampleRate()))
                break;
            d->device = std::move(dev);
            AbstractOutputContext::setDevice(d->device.get());
            d->postSetDevice();
            connect(d->driver, &talcs::AudioDriver::deviceChanged, this, [d] {
                d->handleDeviceHotPlug();
            });
            return true;
        } while (false);
        return enumerateDevices();
    }

    AudioDriverManager *OutputContext::driverManager() const {
        Q_D(const OutputContext);
        return d->driverManager.get();
    }

    AudioDriver *OutputContext::driver() const {
        Q_D(const OutputContext);
        return d->driver;
    }

    bool OutputContext::setDriver(const QString &driverName) {
        Q_D(OutputContext);
        if (d->driver && driverName == d->driver->name() && d->driver->isInitialized() && d->device && d->device->isOpen())
            return true;
        d->device.reset();
        if (d->driver) {
            d->driver->finalize();
            disconnect(d->driver, nullptr, this, nullptr);
        }
        d->driver = d->driverManager->driver(driverName);
        if (!d->driver || !d->driver->initialize()) {
            d->driver = nullptr;
            return false;
        }
        return enumerateDevices();
    }

    bool OutputContext::setDevice(const QString &deviceName) {
        Q_D(OutputContext);
        std::unique_ptr<talcs::AudioDevice> dev;
        if (deviceName.isEmpty())
            dev.reset(d->driver->createDefaultDevice());
        else
            dev.reset(d->driver->createDevice(deviceName));
        if (!dev || !dev->isInitialized()) {
            return false;
        }
        auto savedBufferSize = d->adoptedBufferSize == 0 ? dev->preferredBufferSize() : d->adoptedBufferSize;
        auto savedSampleRate = qFuzzyIsNull(d->adoptedSampleRate) ? dev->preferredSampleRate() : d->adoptedSampleRate;
        if (!dev->open(savedBufferSize, savedSampleRate) && !dev->open(dev->preferredBufferSize(), dev->preferredSampleRate())) {
            return false;
        }
        d->device = std::move(dev);
        AbstractOutputContext::setDevice(d->device.get());
        d->postSetDevice();
        return true;
    }

    bool OutputContext::enumerateDevices() {
        Q_D(OutputContext);
        for (int i = 0;; i++) {
            if (d->driver && d->driver->initialize()) {
                break;
            }
            if (i >= d->driverManager->drivers().size()) {
                d->driver = nullptr;
                return false;
            }
            d->driver = d->driverManager->driver(d->driverManager->drivers()[i]);
            if (d->driver && d->driver->initialize()) {
                break;
            }
        }
        for (int i = -1;; i++) {
            std::unique_ptr<talcs::AudioDevice> currentDevice;
            if (i >= d->driver->devices().size()) {
                return false;
            }
            if (i == -1) {
                if (!d->driver->defaultDevice().isEmpty())
                    currentDevice.reset(d->driver->createDefaultDevice());
            } else {
                currentDevice.reset(d->driver->createDevice(d->driver->devices()[i]));
            }
            if (!currentDevice || !currentDevice->isInitialized())
                continue;
            auto savedBufferSize = d->adoptedBufferSize == 0 ? currentDevice->preferredBufferSize() : d->adoptedBufferSize;
            auto savedSampleRate = qFuzzyIsNull(d->adoptedSampleRate) ? currentDevice->preferredSampleRate() : d->adoptedSampleRate;
            if (!currentDevice->open(savedBufferSize, savedSampleRate) && !currentDevice->open(currentDevice->preferredBufferSize(), currentDevice->preferredSampleRate()))
                continue;
            d->device = std::move(currentDevice);
            AbstractOutputContext::setDevice(d->device.get());
            d->postSetDevice();
            connect(d->driver, &talcs::AudioDriver::deviceChanged, this, [d] {
                d->handleDeviceHotPlug();
            });
            return true;
        }
    }

    qint64 OutputContext::adoptedBufferSize() const {
        Q_D(const OutputContext);
        return d->adoptedBufferSize;
    }

    bool OutputContext::setAdoptedBufferSize(qint64 bufferSize) {
        Q_D(OutputContext);
        if (bufferSize == d->adoptedBufferSize)
            return true;
        d->adoptedBufferSize = bufferSize;
        if (d->device && d->device->isOpen()) {
            if (!d->device->open(d->adoptedBufferSize, d->adoptedSampleRate))
                return false;
        }
        emit bufferSizeChanged(bufferSize);
        controlMixer()->open(d->adoptedBufferSize, d->adoptedSampleRate);
        if (d->device && d->device->isOpen())
            d->device->start(playback());
        return true;
    }

    double OutputContext::adoptedSampleRate() const {
        Q_D(const OutputContext);
        return d->adoptedSampleRate;
    }

    bool OutputContext::setAdoptedSampleRate(double sampleRate) {
        Q_D(OutputContext);
        if (qFuzzyCompare(sampleRate, d->adoptedSampleRate))
            return true;
        d->adoptedSampleRate = sampleRate;
        if (d->device && d->device->isOpen()) {
            if (!d->device->open(d->adoptedBufferSize, d->adoptedSampleRate))
                return false;
            d->device->start(playback());
        }
        emit sampleRateChanged(sampleRate);
        controlMixer()->open(d->adoptedBufferSize, d->adoptedSampleRate);
        return true;
    }

    OutputContext::HotPlugNotificationMode OutputContext::hotPlugNotificationMode() const {
        Q_D(const OutputContext);
        return d->hotPlugNotificationMode;
    }

    void OutputContext::setHotPlugNotificationMode(OutputContext::HotPlugNotificationMode mode) {
        Q_D(OutputContext);
        d->hotPlugNotificationMode = mode;
    }

    void OutputContextPrivate::handleDeviceHotPlug() {
        Q_Q(OutputContext);
        auto deviceList = driver->devices();
        if (device && deviceList.contains(device->name()) && !device->isOpen()) {
            auto savedBufferSize = adoptedBufferSize == 0 ? device->preferredBufferSize() : adoptedBufferSize;
            auto savedSampleRate = qFuzzyIsNull(adoptedSampleRate) ? device->preferredSampleRate() : adoptedSampleRate;
            if (!device->open(savedBufferSize, savedSampleRate) && !device->open(device->preferredBufferSize(), device->preferredSampleRate())) {
                qWarning() << "OutputContext: Tried to reopen current device after hot-plugged, but failed.";
            }
            postSetDevice();
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
        if (adoptedBufferSize != device->bufferSize()) {
            adoptedBufferSize = device->bufferSize();
            emit q->bufferSizeChanged(adoptedBufferSize);
        }
        if (!qFuzzyCompare(adoptedSampleRate, device->sampleRate())) {
            adoptedSampleRate = device->sampleRate();
            emit q->sampleRateChanged(adoptedSampleRate);
        }
        q->controlMixer()->open(adoptedBufferSize, adoptedSampleRate);
        device->start(q->playback());
        emit q->deviceChanged();
    }
}