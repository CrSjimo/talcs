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

#include "MemoryAudioSource.h"
#include "MemoryAudioSource_p.h"

#include "buffer/IAudioSampleContainer.h"

namespace talcs {

    /**
     * @class MemoryAudioSource
     * @brief Produces audio from an IAudioSampleProvider object.
     */

    /**
     * Constructor.
     * @param buffer the IAudioSampleProvider object to get audio from
     * @param takeOwnership if set to @c true, the object will be deleted on deconstruction.
     */
    MemoryAudioSource::MemoryAudioSource(IAudioSampleProvider *buffer, bool takeOwnership)
        : MemoryAudioSource(*new MemoryAudioSourcePrivate) {
        Q_D(MemoryAudioSource);
        d->buffer = buffer;
        d->takeOwnership = takeOwnership;
    }

    MemoryAudioSource::MemoryAudioSource(MemoryAudioSourcePrivate &d) : PositionableAudioSource(d) {
    }

    MemoryAudioSource::~MemoryAudioSource() {
        Q_D(MemoryAudioSource);
        if (d->takeOwnership) {
            delete d->buffer;
        }
    }

    /**
     * Gets the IAudioSampleProvider object used.
     */
    IAudioSampleProvider *MemoryAudioSource::buffer() const {
        Q_D(const MemoryAudioSource);
        return d->buffer;
    }

    /**
     * Sets a new IAudioSampleProvider to use.
     *
     * The ownership of the previous object is no longer taken and the read position will be set to zero.
     * @see MemoryAudioSource()
     */
    IAudioSampleProvider *MemoryAudioSource::setBuffer(IAudioSampleProvider *newBuffer, bool takeOwnership) {
        Q_D(MemoryAudioSource);
        QMutexLocker locker(&d->mutex);
        auto oldBuffer = d->buffer;
        d->buffer = newBuffer;
        d->takeOwnership = takeOwnership;
        d->position = 0;
        return oldBuffer;
    }

    qint64 MemoryAudioSource::read(const AudioSourceReadData &readData) {
        Q_D(MemoryAudioSource);
        QMutexLocker locker(&d->mutex);
        auto bufferLength = length();
        auto channelCount = std::min(d->buffer->channelCount(), readData.buffer->channelCount());
        auto readLength = std::min(readData.length, bufferLength - nextReadPosition());
        for (int i = 0; i < channelCount; i++) {
            readData.buffer->setSampleRange(i, readData.startPos, readLength, *d->buffer, i, d->position);
        }
        d->position += readLength;
        return readLength;
    }

    qint64 MemoryAudioSource::length() const {
        Q_D(const MemoryAudioSource);
        return d->buffer->sampleCount();
    }

    void MemoryAudioSource::setNextReadPosition(qint64 pos) {
        Q_D(MemoryAudioSource);
        QMutexLocker locker(&d->mutex);
        PositionableAudioSource::setNextReadPosition(pos);
    }

    bool MemoryAudioSource::isDuplicatable() const {
        Q_D(const MemoryAudioSource);
        return d->buffer->isDuplicatable();
    }

    DuplicatableObject *MemoryAudioSource::duplicate() const {
        Q_D(const MemoryAudioSource);
        auto newBuf = d->buffer->duplicate();
        if (newBuf)
            return new MemoryAudioSource(static_cast<IAudioSampleProvider *>(newBuf), d->takeOwnership);
        return nullptr;
    }
}