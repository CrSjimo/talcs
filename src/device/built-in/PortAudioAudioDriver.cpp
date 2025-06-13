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

#include "PortAudioAudioDriver_p.h"
#include "PortAudioAudioDevice_p.h"

#include <QDebug>

namespace talcs {

    PortAudioAudioDriver::PortAudioAudioDriver(QObject *parent) : AudioDriver(*new PortAudioAudioDriverPrivate, parent) {}

    PortAudioAudioDriver::~PortAudioAudioDriver() {
        PortAudioAudioDriver::finalize();
    }

    bool PortAudioAudioDriver::initialize() {
        Q_D(PortAudioAudioDriver);
        while (Pa_Terminate() != paNotInitialized) {}
        auto err = Pa_Initialize();
        if (err != paNoError) {
            setErrorString(QString::fromUtf8(Pa_GetErrorText(err)));
            return false;
        }
        d->hostApiInfo = Pa_GetHostApiInfo(d->hostApiIndex);
        return true;
    }

    void PortAudioAudioDriver::finalize() {
        Pa_Terminate();
    }

    QStringList PortAudioAudioDriver::devices() const {
        Q_D(const PortAudioAudioDriver);
        QStringList deviceNames;
        for (PaHostApiIndex i = 0; i < d->hostApiInfo->deviceCount; i++) {
            auto index = Pa_HostApiDeviceIndexToDeviceIndex(d->hostApiIndex, i);
            const PaDeviceInfo *deviceInfo = Pa_GetDeviceInfo(index);
            deviceNames.append(QString::fromUtf8(deviceInfo->name));
        }
        return deviceNames;
    }

    QString PortAudioAudioDriver::defaultDevice() const {
        Q_D(const PortAudioAudioDriver);
        auto deviceInfo = Pa_GetDeviceInfo(d->hostApiInfo->defaultOutputDevice);
        return QString::fromUtf8(deviceInfo->name);
    }

    AudioDevice *PortAudioAudioDriver::createDefaultDevice() {
        return new PortAudioAudioDevice(defaultDevice(), this);
    }

    AudioDevice *PortAudioAudioDriver::createDevice(const QString &name) {
        return new PortAudioAudioDevice(name, this);
    }

    QList<PortAudioAudioDriver *> PortAudioAudioDriver::getDrivers() {
        auto count = Pa_GetHostApiCount();
        QList<PortAudioAudioDriver *> drivers;
        for (PaHostApiIndex i = 0; i < count; i++) {
            const PaHostApiInfo *info = Pa_GetHostApiInfo(i);
            auto driver = new PortAudioAudioDriver;
            driver->setName(QString::fromUtf8(info->name) + " (PortAudio)");
            driver->d_func()->hostApiIndex = i;
            driver->d_func()->hostApiInfo = info;
            if (info->type == paWASAPI) {
                // Note: Override PortAudio's default MME driver and prefer WASAPI driver and create both exclusive and shared drivers.
                drivers.prepend(driver);
                driver = new PortAudioAudioDriver;
                driver->setName(QString::fromUtf8(info->name) + " Exclusive (PortAudio)");
                driver->d_func()->hostApiIndex = i;
                driver->d_func()->hostApiInfo = info;
                driver->d_func()->wasapiExclusive = true;
                drivers.insert(1, driver);
            } else {
                drivers.append(driver);
            }
            
        }
        return drivers;
    }

    bool PortAudioAudioDriver::globalInitialize() {
        auto err = Pa_Initialize();
        if (err != paNoError) {
            qWarning() << "PortAudio initialization failed:" << Pa_GetErrorText(err);
            return false;
        }
        return true;
    }

    void PortAudioAudioDriver::globalFinalize() {
        auto err = Pa_Terminate();
        if (err != paNoError) {
            qWarning() << "PortAudio termination failed:" << Pa_GetErrorText(err);
        }
    }

}
