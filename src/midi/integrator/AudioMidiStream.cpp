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

#include <algorithm>
#include <cmath>

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
        if (d->filter.loadRelaxed())
            d->filter.loadRelaxed()->close();
        if (bufferSize <= 0 || !std::isfinite(sampleRate) || sampleRate <= 0)
            return false;
        if (d->filter.loadRelaxed() && !d->filter.loadRelaxed()->open(bufferSize, sampleRate)) {
            d->filter.loadRelaxed()->close();
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
        if (!isOpen() || readData.length <= 0)
            return 0;
        QList<IntegratedMidiMessage> events;
        events.reserve(midiEvents.size());
        for (auto event : midiEvents) {
            if (event.position >= readData.length || (event.type == MidiInputEvent::Message && !event.message.isValid()))
                continue;
            event.position = std::max<qint64>(0, event.position);
            events.append(event);
        }
        std::stable_sort(events.begin(), events.end(), [](const auto &left, const auto &right) { return left.position < right.position; });
        qint64 l = std::clamp(processReading(readData, events), qint64{0}, readData.length);
        if (!d->filter) return l;
        QMutexLocker locker(&d->filterMutex);
        if (d->filter.loadRelaxed()) {
            d->filter.loadRelaxed()->read({readData.buffer, readData.startPos, l, readData.silentFlags}, events);
        }
        return l;
    }

    bool AudioMidiStream::setReadingFilter(AudioMidiStream *filter) {
        Q_D(AudioMidiStream);
        QMutexLocker locker(&d->filterMutex);
        if (d->filter.loadRelaxed() == filter)
            return true;
        for (auto current = filter; current; current = current->readingFilter()) {
            if (current == this)
                return false;
        }
        if (isOpen() && filter && !filter->open(bufferSize(), sampleRate())) {
            filter->close();
            return false;
        }
        if (d->filter.loadRelaxed())
            d->filter.loadRelaxed()->close();
        d->filter = filter;
        return true;
    }

    AudioMidiStream *AudioMidiStream::readingFilter() const {
        Q_D(const AudioMidiStream);
        return d->filter;
    }
} // talcs
