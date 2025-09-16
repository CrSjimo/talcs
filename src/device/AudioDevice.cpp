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

#include "AudioDevice.h"
#include "AudioDevice_p.h"

#include <TalcsDevice/AudioDriver.h>

namespace talcs {

    AudioDevicePrivate::~AudioDevicePrivate() = default;

    /**
     * @class AudioDevice
     * @brief Base class for audio devices
     */

    /**
     * The constructor should be hidden and only accessible from corresponding AudioDriver object under most
     * circumstances.
     */
    AudioDevice::AudioDevice(QObject *parent) : AudioDevice(*new AudioDevicePrivate, parent) {
    }

    AudioDevice::~AudioDevice() = default;

    AudioDevice::AudioDevice(AudioDevicePrivate &d, QObject *parent) : QObject(parent), d_ptr(&d) {
        d.q_ptr = this;
    }

    /**
     * Gets the driver underlying this device.
     */
    AudioDriver *AudioDevice::driver() const {
        Q_D(const AudioDevice);
        return d->driver;
    }

    /**
     * Gets the number of channels of this device.
     *
     * Note that this function will not work until this device is opened.
     * @see open()
     */
    int AudioDevice::channelCount() const {
        Q_D(const AudioDevice);
        return d->channelCount;
    }

    /**
     * Gets the number of active channels of this device. The return value is less or equal the number of all channels.
     *
     * Note that this function will not work until this device is opened.
     * @see open(), channelCount()
     */
    int AudioDevice::activeChannelCount() const {
        Q_D(const AudioDevice);
        return (d->activeChannelCount < 0 || d->activeChannelCount > d->channelCount) ? d->channelCount
                                                                                      : d->activeChannelCount;
    }

    /**
     * Sets the maximum number of active channels of this device.
     *
     * Note that this function must be invoked when this device is closed. Invoking this function while this device is
     * opened is undefined behavior.
     */
    void AudioDevice::setActiveChannelCount(int num) {
        Q_D(AudioDevice);
        d->activeChannelCount = num;
    }

    /**
     * Gets all buffer sizes that this device supports.
     */
    QList<qint64> AudioDevice::availableBufferSizes() const {
        Q_D(const AudioDevice);
        return d->availableBufferSizes;
    }

    /**
     * Gets the most suitable buffer size for this device.
     */
    qint64 AudioDevice::preferredBufferSize() const {
        Q_D(const AudioDevice);
        return d->preferredBufferSize;
    }

    /**
     * Gets all sample rates that this device supports.
     */
    QList<double> AudioDevice::availableSampleRates() const {
        Q_D(const AudioDevice);
        return d->availableSampleRates;
    }

    /**
     * Gets the most suitable sample rate for this device.
     */
    double AudioDevice::preferredSampleRate() const {
        Q_D(const AudioDevice);
        return d->preferredSampleRate;
    }

    /**
     * Sets the driver underlying this device in the constructor.
     */
    void AudioDevice::setDriver(AudioDriver *driver) {
        Q_D(AudioDevice);
        d->driver = driver;
    }

    /**
     * Sets the number of channel after the device is opened.
     */
    void AudioDevice::setChannelCount(int channelCount) {
        Q_D(AudioDevice);
        d->channelCount = channelCount;
    }

    /**
     * Sets all buffer sizes that this device supports in the constructor.
     * @see availableBufferSizes()
     */
    void AudioDevice::setAvailableBufferSizes(const QList<qint64> &bufferSizes) {
        Q_D(AudioDevice);
        d->availableBufferSizes = bufferSizes;
    }

    /**
     * Sets the most suitable buffer size for this device in the constructor.
     * @see preferredBufferSize()
     */
    void AudioDevice::setPreferredBufferSize(qint64 bufferSize) {
        Q_D(AudioDevice);
        d->preferredBufferSize = bufferSize;
    }

    /**
     * Sets all sample rates that this device supports in the constructor.
     * @see availableBufferSizes()
     */
    void AudioDevice::setAvailableSampleRates(const QList<double> &sampleRates) {
        Q_D(AudioDevice);
        d->availableSampleRates = sampleRates;
    }

    /**
     * Sets the most suitable sample rate for this device in the constructor.
     * @see preferredBufferSize()
     */
    void AudioDevice::setPreferredSampleRate(double sampleRate) {
        Q_D(AudioDevice);
        d->preferredSampleRate = sampleRate;
    }

    /**
     * Starts the device.
     *
     * Only after the device is opened can this function be invoked.
     *
     * Note that the ownership of AudioDeviceCallback object is not taken.
     * @param audioDeviceCallback the callback for streaming audio data
     * @return true if started successfully
     */
    bool AudioDevice::start(AudioDeviceCallback *audioDeviceCallback) {
        Q_D(AudioDevice);
        return d->isStarted = true;
    }

    /**
     * Returns @c true if the device is started.
     */
    bool AudioDevice::isStarted() const {
        Q_D(const AudioDevice);
        return d->isStarted;
    }

    /**
     * Stops the device.
     */
    void AudioDevice::stop() {
        Q_D(AudioDevice);
        d->isStarted = false;
    }

    /**
     * Locks the audio callback.
     *
     * Only lightweight work should be done while the audio device is locked. If any heavy work is going to be done,
     * stop the audio device first, do the work, and start it again.
     *
     * The audio device must be unlocked when it is going to stop or close, otherwise these functions will be blocked.
     */
    void AudioDevice::lock() {
    }

    /**
     * Unlock the audio callback.
     */
    void AudioDevice::unlock() {
    }

    /**
     * Returns @c true if the device has been successfully initialized.
     *
     * The initialization state must be checked before opening the device.
     */
    bool AudioDevice::isInitialized() const {
        Q_D(const AudioDevice);
        return d->isInitialized;
    }

    /**
     * Sets the initialization state in the constructor.
     * @see isInitialized()
     */
    void AudioDevice::setIsInitialized(bool isInitialized) {
        Q_D(AudioDevice);
        d->isInitialized = isInitialized;
    }

    /**
     * Opens the control panel of this device.
     * @return @c true if opened successfully, or @c false if the function fails or this device does not have control
     * panel.
     */
    bool AudioDevice::openControlPanel() {
        return false;
    }

    void AudioDevice::close() {
        AudioStreamBase::close();
    }

    /**
     * @fn void AudioDevice::closed()
     * Emitted when the audio device is closed automatically due to internal reasons.
     */
     
}
