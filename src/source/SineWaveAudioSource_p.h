#ifndef TALCS_SINEWAVEAUDIOSOURCE_P_H
#define TALCS_SINEWAVEAUDIOSOURCE_P_H

#include "SineWaveAudioSource.h"
#include "PositionableAudioSource_p.h"

#include <QMutex>

namespace talcs {
    class SineWaveAudioSourcePrivate : public PositionableAudioSourcePrivate {
        Q_DECLARE_PUBLIC(SineWaveAudioSource);

    public:
        double freq = 0;
        QMutex mutex;
    };
}

#endif // TALCS_SINEWAVEAUDIOSOURCE_P_H
