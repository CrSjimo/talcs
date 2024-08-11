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

#ifndef TALCS_DSPXSINGINGCLIPCONTEXT_P_H
#define TALCS_DSPXSINGINGCLIPCONTEXT_P_H

#include <memory>

#include <QVariant>

#include <TalcsCore/AudioSourceClipSeries.h>

#include <TalcsDspx/DspxSingingClipContext.h>

namespace talcs {
    class DspxSingingClipContextPrivate {
        Q_DECLARE_PUBLIC(DspxSingingClipContext)
    public:
        DspxSingingClipContext *q_ptr;

        AudioSourceClipSeries::ClipView clipView;

        std::unique_ptr<AudioSourceClipSeries> noteClipSeries;
        std::unique_ptr<PositionableMixerAudioSource> controlMixer;

        DspxPseudoSingerContext *pseudoSingerContext;

        int startTick = 0;
        int clipStartTick = 0;
        int clipLenTick = 0;

        QVariant data;

        QMap<int, DspxNoteContext *> notes;
    };
}

#endif //TALCS_DSPXSINGINGCLIPCONTEXT_P_H
