#ifndef TALCS_POSITIONABLEAUDIOSOURCE_H
#define TALCS_POSITIONABLEAUDIOSOURCE_H

#include "AudioSource.h"

namespace talcs {
    class PositionableAudioSourcePrivate;

    class TALCS_EXPORT PositionableAudioSource : public AudioSource {
        Q_DECLARE_PRIVATE(PositionableAudioSource)
    public:
        PositionableAudioSource();
        ~PositionableAudioSource() override;
        virtual qint64 length() const = 0;
        virtual qint64 nextReadPosition() const;
        virtual void setNextReadPosition(qint64 pos);

    protected:
        explicit PositionableAudioSource(PositionableAudioSourcePrivate &d);
    };

    class PositionableAudioSourceStateSaverPrivate;

    class TALCS_EXPORT PositionableAudioSourceStateSaver {
    public:
        explicit PositionableAudioSourceStateSaver(PositionableAudioSource *src);
        ~PositionableAudioSourceStateSaver();

    private:
        QScopedPointer<PositionableAudioSourceStateSaverPrivate> d;
    };
}

#endif // TALCS_POSITIONABLEAUDIOSOURCE_H
