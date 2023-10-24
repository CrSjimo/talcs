#ifndef TALCS_TRANSPORTAUDIOSOURCE_P_H
#define TALCS_TRANSPORTAUDIOSOURCE_P_H

#include <TalcsCore/TransportAudioSource.h>
#include <TalcsCore/private/AudioSource_p.h>

#include <QMutex>

namespace talcs {
    class TransportAudioSourcePrivate : public AudioSourcePrivate {
        Q_DECLARE_PUBLIC(TransportAudioSource)
    public:
        PositionableAudioSource *src = nullptr;
        bool takeOwnership = false;

        qint64 position = 0;
        bool isPlaying = false;
        qint64 loopingStart = -1;
        qint64 loopingEnd = -1;
        QMutex mutex;
        QAtomicInt bufferingCounter = 0;

        void _q_positionAboutToChange(qint64 pos);
    };

    class TransportAudioSourceStateSaverPrivate {
    public:
        TransportAudioSource *src;
        qint64 position;
        QPair<qint64, qint64> loopingRange;
    };
}

#endif // TALCS_TRANSPORTAUDIOSOURCE_P_H
