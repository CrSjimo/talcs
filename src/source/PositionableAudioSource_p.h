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

    class PositionableAudioSourceStateSaverPrivate {
    public:
        PositionableAudioSource *src;
        qint64 position;
    };
}

#endif // TALCS_POSITIONABLEAUDIOSOURCE_P_H
