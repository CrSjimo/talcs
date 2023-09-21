#ifndef TALCS_FUTUREAUDIOSOURCE_P_H
#define TALCS_FUTUREAUDIOSOURCE_P_H

#include "FutureAudioSource.h"
#include "source/PositionableAudioSource_p.h"
#include <QFutureWatcher>

namespace talcs {
    class FutureAudioSourcePrivate: public PositionableAudioSourcePrivate {
        Q_DECLARE_PUBLIC(FutureAudioSource)
    public:
        QFutureWatcher<PositionableAudioSource *> futureWatcher;
        FutureAudioSourceCallbacks callbacks;
        void _q_statusChanged(FutureAudioSource::Status status);
        PositionableAudioSource *src = nullptr;
        QMutex mutex;
    };
}

#endif // TALCS_FUTUREAUDIOSOURCE_P_H
