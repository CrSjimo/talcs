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

#include <atomic>

#include <QHash>
#include <QList>
#include <QPair>

#include <TalcsCore/NoteSynthesizer.h>
#include <TalcsCore/TakeOwnershipPointer.h>


namespace talcs {
    class MidiNoteSynthesizerPrivate : public NoteSynthesizerDetector {
        Q_DECLARE_PUBLIC(MidiNoteSynthesizer)
    public:
        MidiNoteSynthesizer *q_ptr;

        TakeOwnershipPointer<NoteSynthesizer> noteSynthesizer;

        std::atomic<double> frequencyOfA{440.0};
        QHash<QPair<quint64, int>, quint64> voiceIds;
        quint64 nextVoiceId{1};

        QList<NoteSynthesizerDetectorMessage> messages;
        qsizetype messageIndex{};

        void detectInterval(qint64 intervalLength) override;
        NoteSynthesizerDetectorMessage nextMessage() override;
    };
}

#endif //TALCS_MIDINOTESYNTHESIZER_P_H
