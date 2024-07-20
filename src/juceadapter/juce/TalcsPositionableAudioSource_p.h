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

#ifndef TALCS_TALCSPOSITIONABLEAUDIOSOURCE_P_H
#define TALCS_TALCSPOSITIONABLEAUDIOSOURCE_P_H

#include <TalcsJuceAdapter/TalcsPositionableAudioSource.h>

#include <TalcsCore/AudioDataWrapper.h>

namespace talcs {
    class TalcsPositionableAudioSourcePrivate {
        Q_DECLARE_PUBLIC(TalcsPositionableAudioSource)
    public:
        TalcsPositionableAudioSource *q_ptr;
        PositionableAudioSource *src;
        bool takeOwnership;
        AudioDataWrapper wrapper = AudioDataWrapper(nullptr, 0, 0);
        QAtomicInteger<bool> isLooping = false;
    };
}

#endif //TALCS_TALCSPOSITIONABLEAUDIOSOURCE_P_H
