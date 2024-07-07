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

#ifndef TALCS_MIDINOTESYNTHESIZER_H
#define TALCS_MIDINOTESYNTHESIZER_H

#include <TalcsMidi/AudioMidiStream.h>

namespace talcs {

    class NoteSynthesizer;

    class MidiNoteSynthesizerPrivate;

    class TALCSMIDI_EXPORT MidiNoteSynthesizer : public AudioMidiStream {
        Q_DECLARE_PRIVATE(MidiNoteSynthesizer)
    public:
        explicit MidiNoteSynthesizer();
        explicit MidiNoteSynthesizer(NoteSynthesizer *noteSynthesizer, bool takeOwnership = false);
        ~MidiNoteSynthesizer() override;

        bool open(qint64 bufferSize, double sampleRate) override;
        void close() override;

        void setFrequencyOfA(double frequency);
        double frequencyOfA() const;

        NoteSynthesizer *noteSynthesizer() const;

    protected:
        qint64 processReading(const AudioSourceReadData &readData, const QList<IntegratedMidiMessage> &midiEvents) override;

        explicit MidiNoteSynthesizer(MidiNoteSynthesizerPrivate &d);
        QScopedPointer<MidiNoteSynthesizerPrivate> d_ptr;
    };

} // talcs

#endif //TALCS_MIDINOTESYNTHESIZER_H
