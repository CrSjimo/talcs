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

    AbstractOutputContext::AbstractOutputContext(QObject *parent) : QObject(parent), d_ptr(new AbstractOutputContextPrivate) {
        Q_D(AbstractOutputContext);
        d->q_ptr = this;

        d->preMixer = std::make_unique<MixerAudioSource>();
        d->controlMixer = std::make_unique<MixerAudioSource>();
        d->playback = std::make_unique<AudioSourcePlayback>(d->controlMixer.get(), false, false);

        d->controlMixer->addSource(d->preMixer.get());
    }

    AbstractOutputContext::~AbstractOutputContext() = default;

    AudioDevice *AbstractOutputContext::device() const {
        Q_D(const AbstractOutputContext);
        return d->device;
    }

    AudioSourcePlayback *AbstractOutputContext::playback() const {
        Q_D(const AbstractOutputContext);
        return d->playback.get();
    }

    MixerAudioSource *AbstractOutputContext::controlMixer() const {
        Q_D(const AbstractOutputContext);
        return d->controlMixer.get();
    }

    MixerAudioSource *AbstractOutputContext::preMixer() const {
        Q_D(const AbstractOutputContext);
        return d->preMixer.get();
    }

    void AbstractOutputContext::setDevice(AudioDevice *device) {
        Q_D(AbstractOutputContext);
        d->device = device;
        emit deviceChanged();
    }
}