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

#include "PositionableAudioSource.h"
#include "PositionableAudioSource_p.h"

namespace talcs {

    /**
     * @class PositionableAudioSource
     * @brief Base class for sources from which audio data can be streamly read, and the source can be repositioned
     */

    /**
     * Default constructor.
     */
    PositionableAudioSource::PositionableAudioSource() : PositionableAudioSource(*new PositionableAudioSourcePrivate) {
    }

    PositionableAudioSource::PositionableAudioSource(PositionableAudioSourcePrivate &d) : AudioSource(d) {
    }

    PositionableAudioSource::~PositionableAudioSource() = default;

    /**
     * Gets the next read position.
     */
    qint64 PositionableAudioSource::nextReadPosition() const {
        Q_D(const PositionableAudioSource);
        return d->position;
    }

    /**
     * Sets the next read position.
     *
     * The new position might be identical to the current position. Such cases should be handled properly.
     *
     * Note that for derived class that reimplement this function, it should be synchronized with read() function.
     */
    void PositionableAudioSource::setNextReadPosition(qint64 pos) {
        Q_D(PositionableAudioSource);
        d->position = pos;
    }

    PositionableAudioSourceStateSaver::PositionableAudioSourceStateSaver(PositionableAudioSource *src)
        : d(new PositionableAudioSourceStateSaverPrivate{src, src ? src->nextReadPosition() : 0}) {
    }

    PositionableAudioSourceStateSaver::~PositionableAudioSourceStateSaver() {
        if (d->src)
            d->src->setNextReadPosition(d->position);
    }
    
}
