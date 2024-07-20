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

#ifndef TALCS_DSPXTRACKCONTEXT_P_H
#define TALCS_DSPXTRACKCONTEXT_P_H

#include <memory>

#include <QVariant>

#include <TalcsCore/AudioSourceClipSeries.h>

#include <TalcsDspx/DspxTrackContext.h>

namespace talcs {
    class DspxTrackContextPrivate {
        Q_DECLARE_PUBLIC(DspxTrackContext)
    public:
        DspxTrackContext *q_ptr;

        std::unique_ptr<AudioSourceClipSeries> clipSeries;
        std::unique_ptr<PositionableMixerAudioSource> trackMixer;
        std::unique_ptr<PositionableMixerAudioSource> controlMixer;

        DspxProjectContext *projectContext = nullptr;

        QVariant data;

        QMap<int, QPair<DspxAudioClipContext *, AudioSourceClipSeries::ClipView>> clips;

    };
}

#endif //TALCS_DSPXTRACKCONTEXT_P_H
