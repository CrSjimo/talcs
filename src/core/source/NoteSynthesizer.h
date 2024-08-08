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

#ifndef TALCS_NOTESYNTHESIZER_H
#define TALCS_NOTESYNTHESIZER_H

#include <TalcsCore/AudioSource.h>

namespace talcs {

    struct NoteSynthesizerDetectorMessage {
        enum SpecialValueNull {
            Null,
        };
        enum SpecialValueAllNotesOff {
            AllNotesOff,
        };
        enum MessageType {
            NoteMessage,
            PitchMessage,
            VolumeMessage,
        };
        enum NoteMessageType {
            NoteOff,
            NoteOn,
            NoteOnIfNotPlaying,
        };

        struct Note {
            Note(SpecialValueAllNotesOff allNotesOff) : frequency(.0), velocity{}, messageType(NoteOff){
            }
            Note(double frequency, NoteMessageType messageType) : frequency(frequency), velocity(1.), messageType(messageType){
            }
            Note(double frequency, double velocity, NoteMessageType messageType) : frequency(frequency), velocity(velocity), messageType(messageType) {
            }

            double frequency;
            double velocity;
            NoteMessageType messageType;
        };
        struct Pitch {
            double deltaPitch;
        };
        struct Volume {
            double volume;
        };
        NoteSynthesizerDetectorMessage(SpecialValueNull null = {}) : position(-1), messageType{}, pitch{} {
        }
        NoteSynthesizerDetectorMessage(qint64 position, Note note) : position(position), messageType(NoteMessage), note(note) {
        }
        NoteSynthesizerDetectorMessage(qint64 position, Pitch pitch) : position(position), messageType(PitchMessage), pitch(pitch) {
        }
        NoteSynthesizerDetectorMessage(qint64 position, Volume volume) : position(position), messageType(VolumeMessage), volume(volume) {
        }

        constexpr bool isNull() noexcept {
            return position == -1;
        }

        qint64 position;
        MessageType messageType;
        union {
            Note note;
            Pitch pitch;
            Volume volume;
        };
    };

    class NoteSynthesizerDetector {
    public:
        virtual void detectInterval(qint64 intervalLength) = 0;
        virtual NoteSynthesizerDetectorMessage nextMessage() = 0;
    };

    class NoteSynthesizerPrivate;

    class TALCSCORE_EXPORT NoteSynthesizer : public AudioSource {
        Q_DECLARE_PRIVATE(NoteSynthesizer)
    public:
        explicit NoteSynthesizer();
        ~NoteSynthesizer() override;

        bool open(qint64 bufferSize, double sampleRate) override;
        void close() override;

        void setAttackTime(qint64 t);
        qint64 attackTime() const;

        void setDecayTime(qint64 t);
        qint64 decayTime() const;

        void setDecayRatio(double ratio);
        double decayRatio() const;

        void setReleaseTime(qint64 t);
        qint64 releaseTime() const;

        enum Generator {
            Sine,
            Square,
            Triangle,
            Sawtooth,
        };

        using GeneratorFunction = std::function<double(double)>;

        void setGenerator(Generator);
        void setGenerator(const GeneratorFunction &);

        void setDetector(NoteSynthesizerDetector *detector);
        NoteSynthesizerDetector *detector() const;

        void flush(bool force = false);

    protected:
        explicit NoteSynthesizer(NoteSynthesizerPrivate &d);
        qint64 processReading(const AudioSourceReadData &readData) override;

    };

}

#endif //TALCS_NOTESYNTHESIZER_H
