#ifndef TALCS_FUTUREAUDIOSOURCE_P_H
#define TALCS_FUTUREAUDIOSOURCE_P_H

#include <QFutureWatcher>

#include <TalcsCore/private/PositionableAudioSource_p.h>
#include <TalcsSynthesis/FutureAudioSource.h>

namespace talcs {

    class FutureAudioSourcePrivate: public PositionableAudioSourcePrivate {
        Q_DECLARE_PUBLIC(FutureAudioSource)
    public:
        QFutureWatcher<PositionableAudioSource *> *futureWatcher;
        FutureAudioSource::Callbacks callbacks;
        void _q_statusChanged(FutureAudioSource::Status status);
        PositionableAudioSource *src = nullptr;
        QMutex mutex;
    };
    
}

#endif // TALCS_FUTUREAUDIOSOURCE_P_H
