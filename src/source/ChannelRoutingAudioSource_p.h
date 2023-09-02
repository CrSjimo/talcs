#ifndef TALCS_CHANNELROUTINGAUDIOSOURCE_P_H
#define TALCS_CHANNELROUTINGAUDIOSOURCE_P_H

#include "ChannelRoutingAudioSource.h"
#include "MixerAudioSource_p.h"

namespace talcs {
    class ChannelRoutingAudioSourcePrivate : public AudioSourcePrivate, public IMixer<AudioSource> {
        Q_DECLARE_PUBLIC(ChannelRoutingAudioSource)
    public:
        AudioBuffer tmpBuf;
    };
}

#endif // TALCS_CHANNELROUTINGAUDIOSOURCE_P_H
