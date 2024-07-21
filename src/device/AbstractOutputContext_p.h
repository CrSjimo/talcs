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

#ifndef TALCS_ABSTRACTOUTPUTCONTEXT_P_H
#define TALCS_ABSTRACTOUTPUTCONTEXT_P_H

#include <memory>

#include <TalcsDevice/AbstractOutputContext.h>

namespace talcs {
    class AbstractOutputContextPrivate {
        Q_DECLARE_PUBLIC(AbstractOutputContext)
    public:
        AbstractOutputContext *q_ptr;

        std::unique_ptr<MixerAudioSource> preMixer;
        std::unique_ptr<MixerAudioSource> controlMixer;
        std::unique_ptr<AudioSourcePlayback> playback;
        AudioDevice *device = nullptr;
    };
}

#endif //TALCS_ABSTRACTOUTPUTCONTEXT_P_H
