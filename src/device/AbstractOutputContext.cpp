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

#include "AbstractOutputContext.h"
#include "AbstractOutputContext_p.h"

#include <TalcsCore/MixerAudioSource.h>
#include <TalcsDevice/AudioSourcePlayback.h>

namespace talcs {

    /**
     * @class AbstractOutputContext
     * @brief A combination of AudioDevice, AudioSourcePlayback, TransportAudioSource, MixerAudioSource for
     * audio output
     */

    /**
     * Constructor.
     */
    AbstractOutputContext::AbstractOutputContext(QObject *parent) : QObject(parent), d_ptr(new AbstractOutputContextPrivate) {
        Q_D(AbstractOutputContext);
        d->q_ptr = this;

        d->preMixer = std::make_unique<MixerAudioSource>();
        d->controlMixer = std::make_unique<MixerAudioSource>();
        d->playback = std::make_unique<AudioSourcePlayback>(d->controlMixer.get(), false, false);

        d->controlMixer->addSource(d->preMixer.get());
    }

    /**
     * Destructor.
     */
    AbstractOutputContext::~AbstractOutputContext() = default;

    /**
     * Gets the device.
     */
    AudioDevice *AbstractOutputContext::device() const {
        Q_D(const AbstractOutputContext);
        return d->device;
    }

    /**
     *
     * Gets the playback.
     */
    AudioSourcePlayback *AbstractOutputContext::playback() const {
        Q_D(const AbstractOutputContext);
        return d->playback.get();
    }

    /**
     * Gets the MixerAudioSource for controlling.
     */
    MixerAudioSource *AbstractOutputContext::controlMixer() const {
        Q_D(const AbstractOutputContext);
        return d->controlMixer.get();
    }

    /**
     * Gets the pre-mixer.
     */
    MixerAudioSource *AbstractOutputContext::preMixer() const {
        Q_D(const AbstractOutputContext);
        return d->preMixer.get();
    }

    /**
     * For derived classes, after successfully initialized the audio device, call this function to
     * set the device.
     */
    void AbstractOutputContext::setDevice(AudioDevice *device) {
        Q_D(AbstractOutputContext);
        d->device = device;
        emit deviceChanged();
    }

    /**
     * @fn void AbstractOutputContext::bufferSizeChanged(qint64 bufferSize)
     * Emitted when the buffer size is changed, after the audio device is opened and before the
     * pre-mixer is opened.
     */

    /**
     * @fn void AbstractOutputContext::sampleRateChanged(double sampleRate)
     * Emitted when the sample rate is changed, after the audio device is opened and before the
     * pre-mixer is opened.
     */

    /**
     * @fn void AbstractOutputContext::deviceChanged()
     * Emitted when the device is changed, after the pre-mixer is opened and the device is started.
     *
     * Note that this signal may be emitted even though the device is actually not changed in some cases.
     */

}
