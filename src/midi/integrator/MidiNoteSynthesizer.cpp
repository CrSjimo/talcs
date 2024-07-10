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

#include "MidiNoteSynthesizer.h"
#include "MidiNoteSynthesizer_p.h"

namespace talcs {
    MidiNoteSynthesizer::MidiNoteSynthesizer() : MidiNoteSynthesizer(new NoteSynthesizer, true) {

    }

    MidiNoteSynthesizer::MidiNoteSynthesizer(NoteSynthesizer *noteSynthesizer, bool takeOwnership) : MidiNoteSynthesizer(*new MidiNoteSynthesizerPrivate) {
        Q_D(MidiNoteSynthesizer);
        d->noteSynthesizer = noteSynthesizer;
        d->takeOwnership = takeOwnership;
        d->noteSynthesizer->setDetector(d);
    }

    MidiNoteSynthesizer::~MidiNoteSynthesizer() {
        Q_D(MidiNoteSynthesizer);
        if (d->takeOwnership)
            delete d->noteSynthesizer;
    }

    bool MidiNoteSynthesizer::open(qint64 bufferSize, double sampleRate) {
        Q_D(MidiNoteSynthesizer);
        if (!d->noteSynthesizer->open(bufferSize, sampleRate))
            return false;
        return AudioMidiStream::open(bufferSize, sampleRate);
    }

    void MidiNoteSynthesizer::close() {
        Q_D(MidiNoteSynthesizer);
        d->noteSynthesizer->close();
        AudioMidiStream::close();
    }

    void MidiNoteSynthesizer::setFrequencyOfA(double frequency) {
        Q_D(MidiNoteSynthesizer);
        d->frequencyOfA = frequency;
    }

    double MidiNoteSynthesizer::frequencyOfA() const {
        Q_D(const MidiNoteSynthesizer);
        return d->frequencyOfA;
    }

    NoteSynthesizer *MidiNoteSynthesizer::noteSynthesizer() const {
        Q_D(const MidiNoteSynthesizer);
        return d->noteSynthesizer;
    }

    void MidiNoteSynthesizerPrivate::detectInterval(qint64 intervalLength) {

    }

    NoteSynthesizerDetectorMessage MidiNoteSynthesizerPrivate::nextMessage() {
        while (midiEventsIterator != midiEvents.cend()) {
            if (midiEventsIterator->message.isSysEx() && *midiEventsIterator->message.getSysExData() == 0xf7)
                return {(midiEventsIterator++)->position, NoteSynthesizerDetectorMessage::AllNotesOff};
            if (midiEventsIterator->message.isNoteOnOrOff()) {
                NoteSynthesizerDetectorMessage ret = {
                    midiEventsIterator->position,
                    MidiMessage::getMidiNoteInHertz(midiEventsIterator->message.getNoteNumber(), frequencyOfA),
                    midiEventsIterator->message.getFloatVelocity(),
                    midiEventsIterator->message.isNoteOn(),
                };
                midiEventsIterator++;
                return ret;
            }
            midiEventsIterator++;
        }
        return NoteSynthesizerDetectorMessage::Null;
    }

    qint64 MidiNoteSynthesizer::processReading(const AudioSourceReadData &readData, const QList<IntegratedMidiMessage> &midiEvents) {
        Q_D(MidiNoteSynthesizer);
        d->midiEvents = midiEvents;
        d->midiEventsIterator = d->midiEvents.cbegin();
        return d->noteSynthesizer->read(readData);
    }

    MidiNoteSynthesizer::MidiNoteSynthesizer(MidiNoteSynthesizerPrivate &d) : d_ptr(&d) {
        d.q_ptr = this;
    }
} // talcs