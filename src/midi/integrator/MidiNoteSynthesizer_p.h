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

#ifndef TALCS_MIDINOTESYNTHESIZER_P_H
#define TALCS_MIDINOTESYNTHESIZER_P_H

#include <TalcsMidi/MidiNoteSynthesizer.h>

#include <QList>

#include <TalcsCore/NoteSynthesizer.h>
#include <TalcsMidi/private/AudioMidiStream_p.h>

namespace talcs {
    class MidiNoteSynthesizerPrivate : public NoteSynthesizerDetector {
        Q_DECLARE_PUBLIC(MidiNoteSynthesizer)
    public:
        MidiNoteSynthesizer *q_ptr;

        NoteSynthesizer *noteSynthesizer;
        bool takeOwnership = false;

        double frequencyOfA = 440.0;

        QList<IntegratedMidiMessage> midiEvents;
        QList<IntegratedMidiMessage>::const_iterator midiEventsIterator;

        void detectInterval(qint64 intervalLength) override;
        NoteSynthesizerDetectorMessage nextMessage() override;
    };
}

#endif //TALCS_MIDINOTESYNTHESIZER_P_H
