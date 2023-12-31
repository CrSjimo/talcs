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

#ifndef TALCS_FUTUREAUDIOSOURCECLIPSERIES_P_H
#define TALCS_FUTUREAUDIOSOURCECLIPSERIES_P_H

#include <QMutex>
#include <QMap>

#include <TalcsCore/private/AudioSourceClipSeries_p.h>
#include <TalcsCore/private/PositionableAudioSource_p.h>
#include <TalcsSynthesis/FutureAudioSourceClipSeries.h>

namespace talcs {

    class FutureAudioSourceClipSeriesPrivate: public PositionableAudioSourcePrivate, public AudioSourceClipSeriesImpl<FutureAudioSourceClip, FutureAudioSourceClipSeries> {
        Q_DECLARE_PUBLIC(FutureAudioSourceClipSeries)
    public:
        explicit FutureAudioSourceClipSeriesPrivate(FutureAudioSourceClipSeries *q);
        QMutex mutex;
        FutureAudioSourceClipSeries::ReadMode readMode = FutureAudioSourceClipSeries::Notify;
        qint64 cachedLengthAvailable = 0;
        qint64 cachedLengthLoaded = 0;
        qint64 cachedClipsLength = 0;
        QMap<qint64, qint64> clipLengthLoadedDict;
        QMap<qint64, bool> clipLengthCachedDict;

        bool isPauseRequiredEmitted = false;

        TransportAudioSource *bufferingTarget = nullptr;

        bool addClip(const FutureAudioSourceClip &clip);
        void removeClip(const FutureAudioSourceClip &clip);
        void clearClips();

        void notifyPause();
        void notifyResume();
        void checkAndNotify(qint64 position, qint64 length);
        void checkAndNotify();
    };
    
}

#endif // TALCS_FUTUREAUDIOSOURCECLIPSERIES_P_H
