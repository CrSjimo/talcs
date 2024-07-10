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
        NoteSynthesizerDetectorMessage(SpecialValueNull null) : position(-1) {
        }
        NoteSynthesizerDetectorMessage(qint64 position, SpecialValueAllNotesOff allNotesOff) : position(position), frequency(.0), isNoteOn(false) {
        }
        NoteSynthesizerDetectorMessage(qint64 position, double frequency, bool isNoteOn) : position(position), frequency(frequency), velocity(1.), isNoteOn(isNoteOn) {
        }
        NoteSynthesizerDetectorMessage(qint64 position, double frequency, double velocity, bool isNoteOn) : position(position), frequency(frequency), velocity(velocity), isNoteOn(isNoteOn) {
        }
        qint64 position;
        double frequency;
        double velocity;
        bool isNoteOn;
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

        void setAttackRate(double rate);
        double attackRate() const;
        void setReleaseRate(double rate);
        double releaseRate() const;

        enum Generator {
            Sine,
            Square,
            Triangle,
            Sawtooth,
        };

        using GeneratorFunction = std::function<double(double, qint64)>;

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
