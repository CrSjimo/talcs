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

#include "AbstractMidiMessageIntegrator.h"
#include "AbstractMidiMessageIntegrator_p.h"

#include <cmath>

#include <QList>

#include <TalcsCore/IAudioSampleContainer.h>

namespace talcs {
    AbstractMidiMessageIntegrator::AbstractMidiMessageIntegrator() : AbstractMidiMessageIntegrator(*new AbstractMidiMessageIntegratorPrivate) {

    }

    AbstractMidiMessageIntegrator::~AbstractMidiMessageIntegrator() {
        Q_D(AbstractMidiMessageIntegrator);
        AbstractMidiMessageIntegrator::close();
    }

    bool AbstractMidiMessageIntegrator::open(qint64 bufferSize, double sampleRate) {
        Q_D(AbstractMidiMessageIntegrator);
        QMutexLocker locker(&d->mutex);
        AudioSource::close();
        if (d->stream)
            d->stream->close();
        if (bufferSize <= 0 || !std::isfinite(sampleRate) || sampleRate <= 0)
            return false;
        if (d->stream && !d->stream->open(bufferSize, sampleRate)) {
            d->stream->close();
            return false;
        }
        if (AudioSource::open(bufferSize, sampleRate))
            return true;
        if (d->stream)
            d->stream->close();
        return false;
    }

    void AbstractMidiMessageIntegrator::close() {
        Q_D(AbstractMidiMessageIntegrator);
        QMutexLocker locker(&d->mutex);
        if (d->stream)
            d->stream->close();
        AudioSource::close();
    }

    bool AbstractMidiMessageIntegrator::setStream(AudioMidiStream *stream, bool takeOwnership) {
        Q_D(AbstractMidiMessageIntegrator);
        QMutexLocker locker(&d->mutex);
        if (d->stream.get() == stream) {
            d->stream.reset(stream, takeOwnership);
            return true;
        }
        if (isOpen() && stream && !stream->open(bufferSize(), sampleRate())) {
            stream->close();
            return false;
        }
        if (d->stream)
            d->stream->close();
        d->stream.reset(stream, takeOwnership);
        return true;
    }

    AudioMidiStream *AbstractMidiMessageIntegrator::stream() const {
        Q_D(const AbstractMidiMessageIntegrator);
        return d->stream;
    }

    qint64 AbstractMidiMessageIntegrator::processReading(const AudioSourceReadData &readData) {
        Q_D(AbstractMidiMessageIntegrator);
        QMutexLocker locker(&d->mutex);
        auto midiEvents = fetch(readData.length);
        if (d->stream)
            return d->stream->read(readData, midiEvents);
        for (int channel = 0; channel < readData.buffer->channelCount(); ++channel)
            readData.buffer->clear(channel, readData.startPos, readData.length);
        return readData.length;
    }

    AbstractMidiMessageIntegrator::AbstractMidiMessageIntegrator(AbstractMidiMessageIntegratorPrivate &d) : AudioSource(d) {

    }
} // talcs
