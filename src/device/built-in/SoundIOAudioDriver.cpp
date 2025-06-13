/******************************************************************************
 * Copyright (c) 2023-2025 CrSjimo                                            *
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

#include "SoundIOAudioDriver_p.h"

#include <QDebug>

#include "SoundIOAudioDevice_p.h"

namespace talcs {

    SoundIOAudioDriver::SoundIOAudioDriver(QObject *parent) : AudioDriver(*new SoundIOAudioDriverPrivate, parent) {
    }

    void SoundIOAudioDriver::timerEvent(QTimerEvent *event) {
        Q_D(SoundIOAudioDriver);
        soundio_flush_events(d->soundio);
    }

    void SoundIOAudioDriverPrivate::handleDeviceChange() {
        Q_Q(SoundIOAudioDriver);
        emit q->aboutToChangeDevice();
        emit q->deviceChanged();
    }

    SoundIOAudioDriver::~SoundIOAudioDriver() {
        SoundIOAudioDriver::finalize();
    }

    bool SoundIOAudioDriver::initialize() {
        Q_D(SoundIOAudioDriver);
        if (d->soundio) {
            // Already initialized
            return AudioDriver::initialize();
        }
        
        d->soundio = soundio_create();
        if (!d->soundio) {
            setErrorString("SoundIO: Failed to create context");
            return false;
        }
        
        int err;
        if (d->backend == SoundIoBackendNone) {
            err = soundio_connect(d->soundio);
        } else {
            err = soundio_connect_backend(d->soundio, d->backend);
        }
        
        if (err) {
            setErrorString(QString("SoundIO: Error connecting: %1").arg(soundio_strerror(err)));
            soundio_destroy(d->soundio);
            d->soundio = nullptr;
            return false;
        }

        d->soundio->userdata = d;
        d->soundio->on_devices_change = [](SoundIo *soundio) {
            static_cast<SoundIOAudioDriverPrivate *>(soundio->userdata)->handleDeviceChange();
        };
        d->timerId = Qt::TimerId{startTimer(0)};

        soundio_flush_events(d->soundio);
        return AudioDriver::initialize();
    }

    void SoundIOAudioDriver::finalize() {
        Q_D(SoundIOAudioDriver);
        if (d->soundio) {
            soundio_disconnect(d->soundio);
            soundio_destroy(d->soundio);
            d->soundio = nullptr;
            killTimer(d->timerId);
            d->timerId = Qt::TimerId::Invalid;
        }
        AudioDriver::finalize();
    }
    QStringList SoundIOAudioDriver::devices() const {
        Q_D(const SoundIOAudioDriver);
        QStringList deviceNames;
        if (d->createVirtualDefaultDevice) {
            deviceNames.append("");
        }
        int count = soundio_output_device_count(d->soundio);
        for (int i = 0; i < count; i++) {
            SoundIoDevice *device = soundio_get_output_device(d->soundio, i);
            if (device && device->is_raw == d->isRaw) {
                deviceNames.append(QString::fromUtf8(device->name));
                soundio_device_unref(device);
            }
        }
        
        return deviceNames;
    }
    QString SoundIOAudioDriver::defaultDevice() const {
        Q_D(const SoundIOAudioDriver);
        if (d->createVirtualDefaultDevice) {
            return "";
        }
        
        int defaultIdx = soundio_default_output_device_index(d->soundio);
        if (defaultIdx < 0)
            return {};
        
        SoundIoDevice *device = soundio_get_output_device(d->soundio, defaultIdx);
        if (!device)
            return {};
        
        QString deviceName = QString::fromUtf8(device->name);
        soundio_device_unref(device);
        
        return deviceName;
    }
    AudioDevice *SoundIOAudioDriver::createDefaultDevice() {
        Q_D(SoundIOAudioDriver);
        if (d->createVirtualDefaultDevice) {
            int defaultIdx = soundio_default_output_device_index(d->soundio);
            if (defaultIdx < 0)
                return nullptr;
            SoundIoDevice *device = soundio_get_output_device(d->soundio, defaultIdx);
            if (!device)
                return nullptr;
            soundio_device_unref(device);
            return new SoundIOAudioDevice({}, this);
        } else {
            auto name = defaultDevice();
            if (!name.isEmpty()) {
                return new SoundIOAudioDevice(defaultDevice(), this);
            }
            return nullptr;
        }
    }
    AudioDevice *SoundIOAudioDriver::createDevice(const QString &name) {
        return new SoundIOAudioDevice(name, this);
    }

    QList<SoundIOAudioDriver *> SoundIOAudioDriver::getDrivers(bool createVirtualDefaultDevice) {
        QList<SoundIOAudioDriver *> drivers;

        static const struct {
            SoundIoBackend backend;
            const char *name;
        } backends[] = {
            {SoundIoBackendJack, "JACK"},
            {SoundIoBackendPulseAudio, "PulseAudio"},
            {SoundIoBackendAlsa, "ALSA"},
            {SoundIoBackendCoreAudio, "CoreAudio"},
            {SoundIoBackendWasapi, "WASAPI"},
        };
        
        for (const auto &backend : backends) {
            if (soundio_have_backend(backend.backend)) {
                auto specificDriver = new SoundIOAudioDriver;
                specificDriver->setName(QString("%1 (libsoundio)").arg(backend.name));
                specificDriver->d_func()->backend = backend.backend;
                specificDriver->d_func()->createVirtualDefaultDevice = createVirtualDefaultDevice;
                drivers.append(specificDriver);
                if (backend.backend == SoundIoBackendWasapi) {
                    auto rawDriver = new SoundIOAudioDriver;
                    rawDriver->setName(QString("%1 Exclusive (libsoundio)").arg(backend.name));
                    rawDriver->d_func()->backend = SoundIoBackendWasapi;
                    rawDriver->d_func()->isRaw = true;
                    drivers.append(rawDriver);
                }
            }
        }
        
        return drivers;
    }

}