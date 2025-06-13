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

#include "AudioDriverManager.h"
#include "AudioDriverManager_p.h"

#include <QDebug>

#ifdef TALCS_USE_FEATURE_SDL
#    include <TalcsDevice/private/SDLAudioDriver_p.h>
#endif

#ifdef TALCS_USE_FEATURE_ASIO
#    include <TalcsDevice/private/ASIOAudioDriver_p.h>
#endif

#ifdef TALCS_USE_FEATURE_PORTAUDIO
#    include <TalcsDevice/private/PortAudioAudioDriver_p.h>
#endif

#ifdef TALCS_USE_FEATURE_LIBSOUNDIO
#    include <TalcsDevice/private/SoundIOAudioDriver_p.h>
#endif

namespace talcs {

    /**
     * @class AudioDriverManager
     * @brief A registry of audio drivers
     *
     * This object is designed for dynamatic registration of audio drivers for DiffScope plugins.
     */

    /**
     * Constructor.
     */
    AudioDriverManager::AudioDriverManager(QObject *parent)
        : AudioDriverManager(*new AudioDriverManagerPrivate, parent) {
    }

    AudioDriverManager::~AudioDriverManager() = default;

    /**
     * Adds audio driver to this manager.
     *
     * Note that the parent of driver object is set to this object.
     * @return @c true if added successfully, @c false if already exists.
     */
    bool AudioDriverManager::addAudioDriver(AudioDriver *driver) {
        Q_D(AudioDriverManager);
        if (!driver) {
            qWarning() << "AudioDriverManager: driver to add is null.";
            return false;
        }
        auto driverName = driver->name();
        driver->setParent(this);
        d->driverDict.append({driverName, driver});
        emit driverAdded(driver);
        return true;
    }

    /**
     * Remove audio driver from this manager.
     *
     * Note that the parent of driver object is set to @c nullptr.
     * @return @c true if removed successfully, @c false if not exists.
     */
    bool AudioDriverManager::removeDriver(AudioDriver *driver) {
        Q_D(AudioDriverManager);
        if (!driver) {
            qWarning() << "AudioDriverManager: driver to remove is null.";
            return false;
        }
        auto driverName = driver->name();
        auto i = d->driverDict.indexOf({driverName, driver});
        if (i == -1) {
            qWarning() << QString("AudioDriverManager: driver '%1' does not exist.").arg(driverName);
            return false;
        }
        driver->setParent(nullptr);
        d->driverDict.removeAt(i);
        emit driverRemoved(driver);
        return true;
    }

    /**
     * Gets the driver with a specified name.
     */
    AudioDriver *AudioDriverManager::driver(const QString &name) const {
        Q_D(const AudioDriverManager);
        for (auto &[drvName, drv]: d->driverDict) {
            if (drvName == name)
                return drv;
        }
        return nullptr;
    }

    /**
     * Gets a list of names of all drivers.
     */
    QStringList AudioDriverManager::drivers() const {
        Q_D(const AudioDriverManager);
        QStringList list;
        for (auto &[name, _]: d->driverDict) {
            list.append(name);
        }
        return list;
    }

    AudioDriverManager::AudioDriverManager(AudioDriverManagerPrivate &d, QObject *parent) : QObject(parent), d_ptr(&d) {
        d.q_ptr = this;
    }

    /**
     * @overload
     */
    AudioDriverManager * AudioDriverManager::createBuiltInDriverManager(QObject *parent) {
        return createBuiltInDriverManager(parent, CreateVirtualDefaultDevice
#ifdef TALCS_USE_FEATURE_LIBSOUNDIO
            | UseSoundIO
#elif defined(TALCS_USE_FEATURE_SDL)
            | UseSDL
#elif defined(TALCS_USE_FEATURE_PORTAUDIO)
            | UsePortAudio
#endif
#ifdef TALCS_USE_FEATURE_ASIO
            | UseASIO
#endif
        );
    }

    /**
     * Creates an audio driver manager with all built-in drivers.
     */
    AudioDriverManager *AudioDriverManager::createBuiltInDriverManager(QObject *parent, BuiltInDriverManagerOption option) {
        auto drvMgr = new AudioDriverManager(parent);
#ifdef TALCS_USE_FEATURE_PORTAUDIO
        if (qgetenv("TALCS_DEVICE_USE_PORTAUDIO") != "0" && qgetenv("TALCS_DEVICE_USE_PORTAUDIO") == "1" || option & UsePortAudio) {
            if (PortAudioAudioDriver::globalInitialize()) {
                for (auto drv : PortAudioAudioDriver::getDrivers()) {
                    drvMgr->addAudioDriver(drv);
                }
                connect(drvMgr, &QObject::destroyed, []() { PortAudioAudioDriver::globalFinalize(); });
            } else {
                qWarning() << "AudioDriverManager: Failed to initialize PortAudio drivers.";
            }
        }
#endif
#ifdef TALCS_USE_FEATURE_SDL
        if (qgetenv("TALCS_DEVICE_USE_SDL") != "0" && qgetenv("TALCS_DEVICE_USE_SDL") == "1" || option & UseSDL) {
            for (auto drv : SDLAudioDriver::getDrivers(option & CreateVirtualDefaultDevice)) {
                drvMgr->addAudioDriver(drv);
            }
        }
#endif
#ifdef TALCS_USE_FEATURE_LIBSOUNDIO
        if (qgetenv("TALCS_DEVICE_USE_LIBSOUNDIO") != "0" && qgetenv("TALCS_DEVICE_USE_LIBSOUNDIO") == "1" || option & UseSoundIO) {
            for (auto drv : SoundIOAudioDriver::getDrivers(option & CreateVirtualDefaultDevice)) {
                drvMgr->addAudioDriver(drv);
            }
        }
#endif
#ifdef TALCS_USE_FEATURE_ASIO
        if (qgetenv("TALCS_DEVICE_USE_ASIO") != "0" && qgetenv("TALCS_DEVICE_USE_ASIO") == "1" || option & UseASIO) {
            drvMgr->addAudioDriver(new ASIOAudioDriver);
        }
#endif
        return drvMgr;
    }

    /**
     * @fn void AudioDriverManager::driverAdded(AudioDriver *driver)
     * Emitted when a new driver is added to the manager.
     * @see addAudioDriver()
     */

    /**
     * @fn void AudioDriverManager::driverAdded(AudioDriver *driver)
     * Emitted when a new driver is removed from the manager.
     * @see removeAudioDriver()
     */
     
}
