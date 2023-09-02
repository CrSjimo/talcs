#ifndef TALCS_POSITIONABLEMIXERAUDIOSOURCE_P_H
#define TALCS_POSITIONABLEMIXERAUDIOSOURCE_P_H

#include "PositionableMixerAudioSource.h"
#include "MixerAudioSource_p.h"
#include "PositionableAudioSource_p.h"

namespace talcs {
    class PositionableMixerAudioSourcePrivate : public PositionableAudioSourcePrivate,
                                                public IMixer<PositionableAudioSource> {
        Q_DECLARE_PUBLIC(PositionableMixerAudioSource)
    public:
        void setNextReadPositionToAll(qint64 pos);
    };
}

#endif // TALCS_POSITIONABLEMIXERAUDIOSOURCE_P_H
