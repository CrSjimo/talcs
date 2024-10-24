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

#include <QMap>
#include <QMutex>

#include <TalcsCore/private/AudioSourceClipSeries_p.h>
#include <TalcsCore/private/PositionableAudioSource_p.h>
#include <TalcsCore/FutureAudioSourceClipSeries.h>

namespace talcs {

    class FutureAudioSourceClipSeriesPrivate: public PositionableAudioSourcePrivate, public IClipSeriesPrivate, public AudioSourceClipSeriesBase<FutureAudioSource, FutureAudioSourceClipSeriesPrivate> {
        Q_DECLARE_PUBLIC(FutureAudioSourceClipSeries)
    public:
        FutureAudioSourceClipSeriesPrivate();
        QMutex mutex;
        FutureAudioSourceClipSeries::ReadMode readMode = FutureAudioSourceClipSeries::Notify;
        qint64 cachedLengthAvailable = 0;
        qint64 cachedLengthLoaded = 0;
        qint64 cachedClipsLength = 0;
        QMap<qint64, qint64> clipLengthLoadedDict;
        QMap<qint64, bool> clipLengthCachedDict;

        bool isPauseRequiredEmitted = false;

        TransportAudioSource *bufferingTarget = nullptr;

        void emitProgressChanged();

        void postAddClip(const ClipInterval &clip);
        void postRemoveClip(const ClipInterval &clip, bool emitSignal = true);
        void preRemoveAllClips();

        void notifyPause();
        void notifyResume();
        enum NotifyPurpose {
            Pause,
            Resume,
        };
        void checkAndNotify(qint64 position, qint64 length, NotifyPurpose purpose);
        void checkAndNotify(NotifyPurpose purpose);
    };
    
}

#endif // TALCS_FUTUREAUDIOSOURCECLIPSERIES_P_H
