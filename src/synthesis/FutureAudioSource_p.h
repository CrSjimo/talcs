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

#ifndef TALCS_FUTUREAUDIOSOURCE_P_H
#define TALCS_FUTUREAUDIOSOURCE_P_H

#include <QFutureWatcher>

#include <TalcsCore/private/PositionableAudioSource_p.h>
#include <TalcsSynthesis/FutureAudioSource.h>

namespace talcs {

    class FutureAudioSourcePrivate: public PositionableAudioSourcePrivate {
        Q_DECLARE_PUBLIC(FutureAudioSource)
    public:
        QFutureWatcher<PositionableAudioSource *> *futureWatcher;
        FutureAudioSource::Callbacks callbacks;
        void _q_statusChanged(FutureAudioSource::Status status);
        PositionableAudioSource *src = nullptr;
        QMutex mutex;
    };
    
}

#endif // TALCS_FUTUREAUDIOSOURCE_P_H
