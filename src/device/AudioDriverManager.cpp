#include "AudioDriverManager.h"
#include "AudioDriverManager_p.h"

#include <QDebug>

#include "built-in/SDLAudioDriver.h"
#ifdef TALCS_USE_FEATURE_ASIO
#    include "built-in/ASIOAudioDriver.h"
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
     * Creates an audio driver manager with all built-in drivers.
     */
    AudioDriverManager *AudioDriverManager::createBuiltInDriverManager(QObject *parent) {
        auto drvMgr = new AudioDriverManager(parent);
        for (auto drv : SDLAudioDriver::getDrivers()) {
            drvMgr->addAudioDriver(drv);
        }
#ifdef TALCS_USE_FEATURE_ASIO
        drvMgr->addAudioDriver(new ASIOAudioDriver);
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
