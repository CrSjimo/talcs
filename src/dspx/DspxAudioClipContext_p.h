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

#ifndef TALCS_DSPXAUDIOCLIPCONTEXT_P_H
#define TALCS_DSPXAUDIOCLIPCONTEXT_P_H

#include <memory>

#include <TalcsCore/AudioSourceClipSeries.h>

#include <TalcsDspx/DspxAudioClipContext.h>

namespace talcs {

    class AbstractAudioFormatIO;

    class DspxAudioClipContextPrivate {
        Q_DECLARE_PUBLIC(DspxAudioClipContext);
    public:
        DspxAudioClipContext *q_ptr;

        std::unique_ptr<PositionableAudioSource> rawSource;
        std::unique_ptr<BufferingAudioSource> contentSource;
        std::unique_ptr<PositionableMixerAudioSource> clipMixer;
        std::unique_ptr<PositionableMixerAudioSource> controlMixer;

        DspxTrackContext *trackContext;

        AudioSourceClipSeries::ClipView clipView;

        int startTick = 0;
        int clipStartTick = 0;
        int clipLenTick = 0;
        QString path;

        QVariant data;

        void handleIO(AbstractAudioFormatIO *io);

    };
}

#endif //TALCS_DSPXAUDIOCLIPCONTEXT_P_H
