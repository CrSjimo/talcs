/******************************************************************************
 * Copyright (c) 2023 CrSjimo                                                 *
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

#ifndef TALCS_POSITIONABLEMIXERAUDIOSOURCE_P_H
#define TALCS_POSITIONABLEMIXERAUDIOSOURCE_P_H

#include <TalcsCore/PositionableMixerAudioSource.h>
#include <TalcsCore/private/IMixer_p.h>
#include <TalcsCore/private/MixerAudioSource_p.h>
#include <TalcsCore/private/PositionableAudioSource_p.h>

namespace talcs {

    class PositionableMixerAudioSourcePrivate : public PositionableAudioSourcePrivate,
                                                public IMixerPrivate<PositionableAudioSource> {
        Q_DECLARE_PUBLIC(PositionableMixerAudioSource)
    public:
        void setNextReadPositionToAll(qint64 pos);
    };
    
}

#endif // TALCS_POSITIONABLEMIXERAUDIOSOURCE_P_H
