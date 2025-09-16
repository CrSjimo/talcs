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

#include "AudioMidiStream.h"
#include "AudioMidiStream_p.h"

namespace talcs {
    AudioMidiStream::AudioMidiStream() : AudioMidiStream(*new AudioMidiStreamPrivate) {
    }

    AudioMidiStream::AudioMidiStream(AudioMidiStreamPrivate &d) : d_ptr(&d) {
        d.q_ptr = this;
    }
    AudioMidiStream::~AudioMidiStream() = default;

    bool AudioMidiStream::open(qint64 bufferSize, double sampleRate) {
        Q_D(AudioMidiStream);
        QMutexLocker locker(&d->filterMutex);
        AudioStreamBase::close();
        if (d->filter.loadRelaxed() && !d->filter.loadRelaxed()->open(bufferSize, sampleRate)) {
            return false;
        }
        return AudioStreamBase::open(bufferSize, sampleRate);
    }

    void AudioMidiStream::close() {
        Q_D(AudioMidiStream);
        QMutexLocker locker(&d->filterMutex);
        if (d->filter.loadRelaxed())
            d->filter.loadRelaxed()->close();
        AudioStreamBase::close();
    }

    qint64 AudioMidiStream::read(const AudioSourceReadData &readData, const QList<IntegratedMidiMessage> &midiEvents) {
        Q_D(AudioMidiStream);
        qint64 l = processReading(readData, midiEvents);
        if (!d->filter) return l;
        QMutexLocker locker(&d->filterMutex);
        if (d->filter.loadRelaxed()) {
            d->filter.loadRelaxed()->read({readData.buffer, readData.startPos, l, readData.silentFlags}, midiEvents);
        }
        return l;
    }

    void AudioMidiStream::setReadingFilter(AudioMidiStream *filter) {
        Q_D(AudioMidiStream);
        QMutexLocker locker(&d->filterMutex);
        d->filter = filter;
        if (isOpen())
            filter->open(bufferSize(), sampleRate());
    }

    AudioMidiStream *AudioMidiStream::readingFilter() const {
        Q_D(const AudioMidiStream);
        return d->filter;
    }
} // talcs
