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

#include <cmath>

namespace talcs {
    MidiNoteSynthesizer::MidiNoteSynthesizer() : MidiNoteSynthesizer(new NoteSynthesizer, true) {

    }

    MidiNoteSynthesizer::MidiNoteSynthesizer(NoteSynthesizer *noteSynthesizer, bool takeOwnership) : MidiNoteSynthesizer(*new MidiNoteSynthesizerPrivate) {
        Q_D(MidiNoteSynthesizer);
        d->noteSynthesizer.reset(noteSynthesizer, takeOwnership);
        if (d->noteSynthesizer)
            d->noteSynthesizer->setDetector(d);
    }

    MidiNoteSynthesizer::~MidiNoteSynthesizer() {
        Q_D(MidiNoteSynthesizer);
        close();
        if (d->noteSynthesizer && d->noteSynthesizer->detector() == d)
            d->noteSynthesizer->setDetector(nullptr);
    }

    bool MidiNoteSynthesizer::open(qint64 bufferSize, double sampleRate) {
        Q_D(MidiNoteSynthesizer);
        d->voiceIds.clear();
        d->messages.clear();
        if (!d->noteSynthesizer || !d->noteSynthesizer->open(bufferSize, sampleRate))
            return false;
        if (AudioMidiStream::open(bufferSize, sampleRate))
            return true;
        d->noteSynthesizer->close();
        return false;
    }

    void MidiNoteSynthesizer::close() {
        Q_D(MidiNoteSynthesizer);
        if (d->noteSynthesizer)
            d->noteSynthesizer->close();
        d->messages.clear();
        d->voiceIds.clear();
        AudioMidiStream::close();
    }

    void MidiNoteSynthesizer::setFrequencyOfA(double frequency) {
        Q_D(MidiNoteSynthesizer);
        if (std::isfinite(frequency) && frequency > 0) {
            d->frequencyOfA.store(frequency);
            if (d->noteSynthesizer)
                d->noteSynthesizer->flush(true);
        }
    }

    double MidiNoteSynthesizer::frequencyOfA() const {
        Q_D(const MidiNoteSynthesizer);
        return d->frequencyOfA.load();
    }

    NoteSynthesizer *MidiNoteSynthesizer::noteSynthesizer() const {
        Q_D(const MidiNoteSynthesizer);
        return d->noteSynthesizer;
    }

    void MidiNoteSynthesizerPrivate::detectInterval(qint64) {

    }

    NoteSynthesizerDetectorMessage MidiNoteSynthesizerPrivate::nextMessage() {
        return messageIndex < messages.size() ? messages.at(messageIndex++) : NoteSynthesizerDetectorMessage::Null;
    }

    qint64 MidiNoteSynthesizer::processReading(const AudioSourceReadData &readData, const QList<IntegratedMidiMessage> &midiEvents) {
        Q_D(MidiNoteSynthesizer);
        d->messages.clear();
        d->messageIndex = 0;
        const auto silence = [&](qint64 position, quint64 voiceId, bool force) {
            NoteSynthesizerDetectorMessage message{position, NoteSynthesizerDetectorMessage::AllNotesOff};
            message.note.voiceId = voiceId;
            if (force)
                message.note.messageType = NoteSynthesizerDetectorMessage::SoundOff;
            d->messages.append(message);
        };
        for (const auto &event : midiEvents) {
            if (event.type == MidiInputEvent::AllSourcesStopped) {
                silence(event.position, 0, true);
                d->voiceIds.clear();
                continue;
            }
            if (event.type == MidiInputEvent::SourceStopped) {
                for (auto it = d->voiceIds.begin(); it != d->voiceIds.end();) {
                    if (it.key().first == event.sourceId) {
                        silence(event.position, it.value(), true);
                        it = d->voiceIds.erase(it);
                    } else {
                        ++it;
                    }
                }
                continue;
            }
            const auto &input = event.message;
            if (!input.isNoteOnOrOff() && !input.isPitchWheel() && !input.isAllNotesOff() && !input.isAllSoundOff())
                continue;
            const auto key = qMakePair(event.sourceId, input.getChannel());
            auto id = d->voiceIds.find(key);
            if (id == d->voiceIds.end())
                id = d->voiceIds.insert(key, d->nextVoiceId++);
            if (input.isAllNotesOff() || input.isAllSoundOff()) {
                silence(event.position, id.value(), input.isAllSoundOff());
            } else if (input.isNoteOnOrOff()) {
                NoteSynthesizerDetectorMessage message{event.position, {
                    MidiMessage::getMidiNoteInHertz(input.getNoteNumber(), d->frequencyOfA.load()),
                    input.getFloatVelocity(),
                    input.isNoteOn() ? NoteSynthesizerDetectorMessage::NoteOn : NoteSynthesizerDetectorMessage::NoteOff}};
                message.note.voiceId = id.value();
                d->messages.append(message);
            } else {
                const auto wheel = input.getPitchWheelValue() - 8192;
                d->messages.append({event.position, NoteSynthesizerDetectorMessage::Pitch{wheel * 2.0 / (wheel < 0 ? 8192.0 : 8191.0), id.value()}});
            }
        }
        return d->noteSynthesizer->read(readData);
    }

    MidiNoteSynthesizer::MidiNoteSynthesizer(MidiNoteSynthesizerPrivate &d) : d_ptr(&d) {
        d.q_ptr = this;
    }
} // talcs
