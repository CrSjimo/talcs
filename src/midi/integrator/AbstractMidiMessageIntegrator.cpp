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

#include <QList>

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
        if (d->stream && !d->stream->open(bufferSize, sampleRate))
            return false;
        return AudioSource::open(bufferSize, sampleRate);
    }

    void AbstractMidiMessageIntegrator::close() {
        Q_D(AbstractMidiMessageIntegrator);
        QMutexLocker locker(&d->mutex);
        if (d->stream)
            d->stream->close();
        AudioSource::close();
    }

    void AbstractMidiMessageIntegrator::setStream(AudioMidiStream *stream, bool takeOwnership) {
        Q_D(AbstractMidiMessageIntegrator);
        QMutexLocker locker(&d->mutex);
        d->stream.reset(stream, takeOwnership);
        if (isOpen())
            d->stream->open(bufferSize(), sampleRate());
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
        return readData.length;
    }

    AbstractMidiMessageIntegrator::AbstractMidiMessageIntegrator(AbstractMidiMessageIntegratorPrivate &d) : AudioSource(d) {

    }
} // talcs