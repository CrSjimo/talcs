#ifndef TALCS_POSITIONABLEMIXERAUDIOSOURCE_P_H
#define TALCS_POSITIONABLEMIXERAUDIOSOURCE_P_H

#include <TalcsCore/private/IMixer_p.h>
#include <TalcsCore/private/MixerAudioSource_p.h>
#include <TalcsCore/private/PositionableAudioSource_p.h>
#include <TalcsCore/PositionableMixerAudioSource.h>

namespace talcs {

    class PositionableMixerAudioSourcePrivate : public PositionableAudioSourcePrivate,
                                                public IMixerPrivate<PositionableAudioSource> {
        Q_DECLARE_PUBLIC(PositionableMixerAudioSource)
    public:
        void setNextReadPositionToAll(qint64 pos);
    };
    
}

#endif // TALCS_POSITIONABLEMIXERAUDIOSOURCE_P_H
