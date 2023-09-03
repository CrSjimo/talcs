#ifndef TALCS_POSITIONABLEAUDIOSOURCE_P_H
#define TALCS_POSITIONABLEAUDIOSOURCE_P_H

#include "AudioSource_p.h"
#include "PositionableAudioSource.h"

namespace talcs {
    class PositionableAudioSourcePrivate : public AudioSourcePrivate {
        Q_DECLARE_PUBLIC(PositionableAudioSource)
    public:
        qint64 position = 0;
    };
}

#endif // TALCS_POSITIONABLEAUDIOSOURCE_P_H
