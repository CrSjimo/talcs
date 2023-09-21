#ifndef TALCS_FUTUREAUDIOSOURCECLIPSERIES_P_H
#define TALCS_FUTUREAUDIOSOURCECLIPSERIES_P_H

#include "FutureAudioSourceClipSeries.h"

#include <QMutex>

#include "source/PositionableAudioSource_p.h"

namespace talcs {
    class FutureAudioSourceClipSeriesPrivate: public PositionableAudioSourcePrivate {
        Q_DECLARE_PUBLIC(FutureAudioSourceClipSeries)
    public:
        QMutex mutex;
        FutureAudioSourceClipSeries::ReadMode readMode = FutureAudioSourceClipSeries::Notify;
    };
}

#endif // TALCS_FUTUREAUDIOSOURCECLIPSERIES_P_H
