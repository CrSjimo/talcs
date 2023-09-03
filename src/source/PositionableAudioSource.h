#ifndef TALCS_POSITIONABLEAUDIOSOURCE_H
#define TALCS_POSITIONABLEAUDIOSOURCE_H

#include "AudioSource.h"

namespace talcs {
    class PositionableAudioSourcePrivate;

    class TALCS_EXPORT PositionableAudioSource : public AudioSource {
        Q_DECLARE_PRIVATE(PositionableAudioSource)
    public:
        PositionableAudioSource();
        virtual qint64 length() const = 0;
        virtual qint64 nextReadPosition() const;
        virtual void setNextReadPosition(qint64 pos);

    protected:
        explicit PositionableAudioSource(PositionableAudioSourcePrivate &d);
    };
}

#endif // TALCS_POSITIONABLEAUDIOSOURCE_H
