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

#ifndef TALCS_SINEWAVEAUDIOSOURCE_P_H
#define TALCS_SINEWAVEAUDIOSOURCE_P_H

#include <QMutex>

#include <TalcsCore/SineWaveAudioSource.h>
#include <TalcsCore/private/PositionableAudioSource_p.h>

namespace talcs {

    class SineWaveAudioSourcePrivate : public PositionableAudioSourcePrivate {
        Q_DECLARE_PUBLIC(SineWaveAudioSource);

    public:
        std::function<double(qint64)> freq = [](qint64 _) { return 0.0; };
        QMutex mutex;
    };

}

#endif // TALCS_SINEWAVEAUDIOSOURCE_P_H
