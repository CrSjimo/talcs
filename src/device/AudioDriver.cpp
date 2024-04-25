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

#include "AudioDriver.h"
#include "AudioDriver_p.h"

namespace talcs {

    /**
     * @class AudioDriver
     * @brief Base class for audio drivers that underlay audio devices
     */

    /**
     * Constructor.
     */
    AudioDriver::AudioDriver(QObject *parent) : AudioDriver(*new AudioDriverPrivate, parent) {
    }

    AudioDriver::AudioDriver(AudioDriverPrivate &d, QObject *parent) : QObject(parent), d_ptr(&d) {
        d.q_ptr = this;
    }

    AudioDriver::~AudioDriver() = default;

    /**
     * Initializes current audio driver.
     *
     * The driver must be initialized first before getting the device list or creating device.
     * @return @c true if the initialization is successful
     */
    bool AudioDriver::initialize() {
        Q_D(AudioDriver);
        return d->isInitialized = true;
    }

    /**
     * Terminates current audio driver.
     *
     * After the driver is terminated, the behavior of created devices is undefined, therefore the life cycle of drivers
     * and devices should be managed properly.
     */
    void AudioDriver::finalize() {
        Q_D(AudioDriver);
        d->isInitialized = false;
    }

    /**
     * Returns @c true if the audio driver is initialized.
     */
    bool AudioDriver::isInitialized() const {
        Q_D(const AudioDriver);
        return d->isInitialized;
    }

    /**
     * @fn QStringList AudioDriver::devices() const
     * Gets the names of devices under this driver.
     */

    /**
     * Gets the name of default device under this driver.
     * @return the device name, or null string if this driver does not support specifying default device
     */
    QString AudioDriver::defaultDevice() const {
        return {};
    }

    /**
     * @fn AudioDevice *AudioDriver::createDevice(const QString &name)
     * Creates an audio device.
     *
     * The parent of the device created is set to this object, and use @c delete to deallocate.
     *
     * @c nullptr will be returned if the function fails to create the device with the specified name. Note that the
     * audio device is not guaranteed to work, and use @link AudioDevice::isInitialized() to check whether it can work
     * or not.
     * @see AudioDevice, AudioDevice::isInitialized()
     */

    /**
     * @fn void AudioDevice::deviceChanged()
     * Emitted when the list of devices is changed. Not all drivers support this feature.
     */
     
}