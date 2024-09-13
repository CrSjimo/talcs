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

#ifndef TALCS_DSPXNOTECONTEXT_P_H
#define TALCS_DSPXNOTECONTEXT_P_H

#include <memory>

#include <QMutex>
#include <QVariant>

#include <TalcsCore/PositionableAudioSource.h>
#include <TalcsCore/AudioSourceClipSeries.h>
#include <TalcsCore/NoteSynthesizer.h>

#include <TalcsDspx/DspxNoteContext.h>

namespace talcs {

    class DspxNoteContextSynthesizer : public talcs::PositionableAudioSource, public talcs::NoteSynthesizerDetector {
    public:
        bool open(qint64 bufferSize, double sampleRate) override;
        void close() override;

        qint64 length() const override;

        void setNextReadPosition(qint64 pos) override;

        void detectInterval(qint64 intervalLength) override;
        NoteSynthesizerDetectorMessage nextMessage() override;

        void update();

        DspxNoteContextPrivate *d;
        QList<NoteSynthesizerDetectorMessage> messages;
        QList<NoteSynthesizerDetectorMessage>::const_iterator messageIterator;
        QMutex mutex;

    protected:
        qint64 processReading(const AudioSourceReadData &readData) override;
    };

    class DspxNoteContextPrivate {
        Q_DECLARE_PUBLIC(DspxNoteContext)
    public:
        DspxNoteContext *q_ptr;

        AudioSourceClipSeries::ClipView clipView;

        std::unique_ptr<NoteSynthesizer> noteSynthesizer;
        std::unique_ptr<DspxNoteContextSynthesizer> synthesizer;

        DspxSingingClipContext *singingClipContext;

        int posTick = 0;
        int lengthTick = 0;

        int keyCent = 0;

        QVariant data;

        QMap<int, QVariant> pitchAnchors;
        QMap<int, QVariant> energyAnchors;
    };
}

#endif //TALCS_DSPXNOTECONTEXT_P_H
