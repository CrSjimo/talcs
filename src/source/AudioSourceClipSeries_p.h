#ifndef TALCS_AUDIOSOURCECLIPSERIES_P_H
#define TALCS_AUDIOSOURCECLIPSERIES_P_H

#include "AudioSourceClipSeries.h"
#include "PositionableAudioSource_p.h"
#include <QMutex>

namespace talcs {
    class AudioSourceClipSeriesPrivate : public PositionableAudioSourcePrivate {
        Q_DECLARE_PUBLIC(AudioSourceClipSeries);

    public:
        QMutex mutex;
    };
}

#endif // TALCS_AUDIOSOURCECLIPSERIES_P_H
