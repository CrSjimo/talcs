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

#ifndef TALCS_AUDIOSOURCE_P_H
#define TALCS_AUDIOSOURCE_P_H

#include <QMutex>

#include <TalcsCore/AudioSource.h>

namespace talcs {

    class TALCSCORE_EXPORT AudioSourcePrivate {
        Q_DECLARE_PUBLIC(AudioSource)
    public:

        virtual ~AudioSourcePrivate();

        AudioSource *q_ptr;

        QMutex filterMutex;
        QAtomicPointer<AudioSource> filter = nullptr;
    };
    
}

#endif // TALCS_AUDIOSOURCE_P_H
