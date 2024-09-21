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

#include "AudioSourcePlayback.h"
#include "AudioSourcePlayback_p.h"
#include "AudioDevice.h"

#include <TalcsCore/AudioSource.h>

namespace talcs {

    /**
     * @class AudioSourcePlayback
     * @brief Callback functions that starts, stops and reads data from an AudioSource
     *
     * In practice, TransportAudioSource is usually used to control the transportation of audio.
     *
     * This is one of the @ref doc/object_binding.md "object-binding" classes.
     *
     * @see AudioSource, TransportAudioSource
     */

    /**
     * Constructor.
     * @param src the AudioSource to process
     * @param takeOwnership If the ownership of the AudioSource object is taken, the object will be deleted on
     * destruction.
     * @param managedByDevice If set to true, the source will be opened on device started and closed on device stopped.
     */
    AudioSourcePlayback::AudioSourcePlayback(AudioSource *src, bool takeOwnership, bool managedByDevice)
        : AudioSourcePlayback(*new AudioSourcePlaybackPrivate) {
        Q_D(AudioSourcePlayback);
        d->src.reset(src, takeOwnership);
        d->managedByDevice = managedByDevice;
    }

    AudioSourcePlayback::AudioSourcePlayback(AudioSourcePlaybackPrivate &d) : d_ptr(&d) {
        d.q_ptr = this;
    }

    /**
     * Destructor.
     *
     * If the ownership of the AudioSource object is taken, it will be deleted.
     */
    AudioSourcePlayback::~AudioSourcePlayback() = default;

    /**
     * Gets the AudioSource object used.
     */
    AudioSource *AudioSourcePlayback::source() const {
        Q_D(const AudioSourcePlayback);
        return d->src;
    }

    /**
     * Sets the AudioSource object used.
     */
    void AudioSourcePlayback::setSource(AudioSource *src, bool takeOwnership, bool managedByDevice) {
        Q_D(AudioSourcePlayback);
        QMutexLocker locker(&d->mutex);
        d->src.reset(src, takeOwnership);
        d->managedByDevice = managedByDevice;
    }

    bool AudioSourcePlayback::deviceWillStartCallback(AudioDevice *device) {
        Q_D(AudioSourcePlayback);
        QMutexLocker locker(&d->mutex);
        if (d->managedByDevice)
            return d->src->open(device->bufferSize(), device->sampleRate());
        else
            return true;
    }
    void AudioSourcePlayback::deviceStoppedCallback() {
        Q_D(AudioSourcePlayback);
        QMutexLocker locker(&d->mutex);
        if (d->managedByDevice)
            d->src->close();
    }

    void AudioSourcePlayback::workCallback(const AudioSourceReadData &readData) {
        Q_D(AudioSourcePlayback);
        QMutexLocker locker(&d->mutex);
        d->src->read(readData);
    }
    
}
