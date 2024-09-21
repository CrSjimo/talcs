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

#ifndef TALCS_METRONOMEAUDIOSOURCE_P_H
#define TALCS_METRONOMEAUDIOSOURCE_P_H

#include <QMutex>

#include <TalcsCore/MetronomeAudioSource.h>
#include <TalcsCore/private/AudioSource_p.h>
#include <TalcsCore/TakeOwnershipPointer.h>

namespace talcs {
    class MetronomeAudioSourcePrivate : public AudioSourcePrivate {
        Q_DECLARE_PUBLIC(MetronomeAudioSource);
    public:
        QMutex mutex;
        TakeOwnershipPointer<PositionableAudioSource> majorBeatSource;
        TakeOwnershipPointer<PositionableAudioSource> minorBeatSource;
        MetronomeAudioSourceDetector *detector = nullptr;

        bool tailIsMajor = false;
        bool tailIsMinor = false;
    };
}

#endif //TALCS_METRONOMEAUDIOSOURCE_P_H
