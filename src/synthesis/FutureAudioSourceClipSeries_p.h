#ifndef TALCS_FUTUREAUDIOSOURCECLIPSERIES_P_H
#define TALCS_FUTUREAUDIOSOURCECLIPSERIES_P_H

#include "FutureAudioSourceClipSeries.h"

#include <QMutex>
#include <QMap>

#include "source/PositionableAudioSource_p.h"
#include "source/AudioSourceClipSeries_p.h"

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

        bool addClip(const FutureAudioSourceClip &clip);
        void removeClip(const FutureAudioSourceClip &clip);
        void clearClips();
    };
}

#endif // TALCS_FUTUREAUDIOSOURCECLIPSERIES_P_H
