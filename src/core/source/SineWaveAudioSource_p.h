#ifndef TALCS_SINEWAVEAUDIOSOURCE_P_H
#define TALCS_SINEWAVEAUDIOSOURCE_P_H

#include <TalcsCore/private/PositionableAudioSource_p.h>
#include <TalcsCore/SineWaveAudioSource.h>

#include <QMutex>

namespace talcs {

    class SineWaveAudioSourcePrivate : public PositionableAudioSourcePrivate {
        Q_DECLARE_PUBLIC(SineWaveAudioSource);

    public:
        std::function<double(qint64)> freq = [](qint64 _) { return 0.0; };
        QMutex mutex;
    };

}

#endif // TALCS_SINEWAVEAUDIOSOURCE_P_H
