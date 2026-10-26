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

#ifdef Q_OS_LINUX
#    include <dlfcn.h>
#endif

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QLibrary>
#include <QLoggingCategory>

#include <TalcsDevice/AudioDriver.h>

#ifdef Q_OS_WIN
#    include <TalcsDevice/private/WASAPIAudioDriver_p.h>
#elif defined(Q_OS_MACOS)
#    include <TalcsDevice/private/CoreAudioAudioDriver_p.h>
#endif

namespace talcs {

    Q_LOGGING_CATEGORY(lcAudioDriverManager, "talcs.device.driverManager")

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
        return d->insertAudioDriver(driver);
    }

    bool AudioDriverManagerPrivate::insertAudioDriver(AudioDriver *driver, AudioDriver *before) {
        Q_Q(AudioDriverManager);
        if (!driver) {
            qCWarning(lcAudioDriverManager) << "Audio driver registration failed because the driver is null.";
            return false;
        }
        auto driverName = driver->name();
        driver->setParent(q);
        const auto index = before ? driverDict.indexOf({before->name(), before}) : driverDict.size();
        driverDict.insert(index, {driverName, driver});
        qCInfo(lcAudioDriverManager) << "Audio driver registered." << "driver:" << driverName << "display name:" << driver->displayName();
        Q_EMIT q->driverAdded(driver);
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
            qCWarning(lcAudioDriverManager) << "Audio driver removal failed because the driver is null.";
            return false;
        }
        auto driverName = driver->name();
        auto i = d->driverDict.indexOf({driverName, driver});
        if (i == -1) {
            qCWarning(lcAudioDriverManager) << "Audio driver removal failed because the driver is not registered." << "driver:" << driverName;
            return false;
        }
        driver->setParent(nullptr);
        d->driverDict.removeAt(i);
        qCInfo(lcAudioDriverManager) << "Audio driver removed." << "driver:" << driverName;
        Q_EMIT driverRemoved(driver);
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
     * Creates an audio driver manager with all built-in drivers.
     */
    AudioDriverManager *AudioDriverManager::createBuiltInDriverManager(QObject *parent) {
        qCInfo(lcAudioDriverManager) << "Built-in audio driver enumeration started.";
        auto drvMgr = new AudioDriverManager(parent);
#ifdef Q_OS_WIN
        drvMgr->addAudioDriver(new WASAPIAudioDriver(WASAPIAudioDriver::Shared));
        drvMgr->addAudioDriver(new WASAPIAudioDriver(WASAPIAudioDriver::Exclusive));
#elif defined(Q_OS_MACOS)
        drvMgr->addAudioDriver(new CoreAudioAudioDriver(CoreAudioAudioDriver::Shared));
        drvMgr->addAudioDriver(new CoreAudioAudioDriver(CoreAudioAudioDriver::Hog));
#elif defined(Q_OS_LINUX)
        drvMgr->loadBuiltInDriverLibraries();
#endif
        qCInfo(lcAudioDriverManager) << "Built-in audio driver enumeration completed." << "drivers:" << drvMgr->drivers();
        if (drvMgr->drivers().isEmpty()) {
            qCWarning(lcAudioDriverManager) << "No built-in audio drivers are available.";
        }
        return drvMgr;
    }

    /**
     * Loads available native driver libraries in preference order from directory or the device library directory.
     */
    void AudioDriverManager::loadBuiltInDriverLibraries(const QString &directory) {
#ifdef Q_OS_LINUX
        Q_D(AudioDriverManager);
        auto libraryDirectory = directory;
        if (libraryDirectory.isEmpty()) {
            Dl_info information{};
            if (dladdr(reinterpret_cast<void *>(&AudioDriverManager::createBuiltInDriverManager), &information) && information.dli_fname) {
                libraryDirectory = QFileInfo(QString::fromLocal8Bit(information.dli_fname)).absolutePath();
            }
        }
        struct Library {
            const char *driver;
            const char *module;
        };
        static const Library libraries[] = {
            {"PipeWire", "TalcsPipeWire"},
            {"PulseAudio", "TalcsPulseAudio"},
            {"JACK", "TalcsJack"},
            {"ALSA", "TalcsAlsa"},
        };
        for (const auto &entry : libraries) {
            if (driver(QString::fromLatin1(entry.driver))) {
                continue;
            }
            auto *library = new QLibrary(this);
            library->setFileName(libraryDirectory.isEmpty() ? QString::fromLatin1(entry.module) : QDir(libraryDirectory).filePath(QStringLiteral("lib%1.so").arg(QString::fromLatin1(entry.module))));
            // Removed drivers may outlive the manager, so their code must remain loaded.
            library->setLoadHints(QLibrary::ResolveAllSymbolsHint | QLibrary::PreventUnloadHint);
            using Factory = AudioDriver *(*)(QObject *);
            const auto factory = reinterpret_cast<Factory>(library->resolve("talcsCreateAudioDriver"));
            if (!factory) {
                qCDebug(lcAudioDriverManager) << "Native audio driver library is unavailable." << library->fileName() << library->errorString();
                delete library;
                continue;
            }
            auto *nativeDriver = factory(this);
            if (!nativeDriver || nativeDriver->name() != QString::fromLatin1(entry.driver)) {
                qCWarning(lcAudioDriverManager) << "Native audio driver factory returned an invalid driver." << library->fileName();
                delete nativeDriver;
                delete library;
                continue;
            }
            AudioDriver *nextDriver = nullptr;
            bool following = false;
            for (const auto &candidate : libraries) {
                if (&candidate == &entry) {
                    following = true;
                } else if (following && (nextDriver = driver(QString::fromLatin1(candidate.driver)))) {
                    break;
                }
            }
            d->insertAudioDriver(nativeDriver, nextDriver);
        }
#else
        Q_UNUSED(directory)
#endif
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
