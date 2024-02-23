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

#ifndef TALCS_TRANSPORTAUDIOSOURCE_P_H
#define TALCS_TRANSPORTAUDIOSOURCE_P_H

#include <QMutex>

#include <TalcsCore/TransportAudioSource.h>
#include <TalcsCore/private/AudioSource_p.h>

namespace talcs {
    class TransportAudioSourcePrivate : public AudioSourcePrivate {
        Q_DECLARE_PUBLIC(TransportAudioSource)
    public:
        PositionableAudioSource *src = nullptr;
        bool takeOwnership = false;

        qint64 position = 0;
        std::atomic<TransportAudioSource::PlaybackStatus> playbackStatus = TransportAudioSource::Paused;
        qint64 loopingStart = -1;
        qint64 loopingEnd = -1;
        QMutex mutex;
        QAtomicInt bufferingCounter = 0;

        void _q_positionAboutToChange(qint64 pos);
    };

    class TransportAudioSourceStateSaverPrivate {
    public:
        TransportAudioSource *src;
        qint64 position;
        QPair<qint64, qint64> loopingRange;
    };
}

#endif // TALCS_TRANSPORTAUDIOSOURCE_P_H
