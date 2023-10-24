#ifndef TALCS_MIXERAUDIOSOURCE_P_H
#define TALCS_MIXERAUDIOSOURCE_P_H

#include "MixerAudioSource.h"

#include <QMutex>

#include <QMChronMap.h>

#include "AudioSource_p.h"
#include "IMixer_p.h"
#include "core/buffer/AudioBuffer.h"

namespace talcs {
    class MixerAudioSourcePrivate : public AudioSourcePrivate, public IMixerPrivate<AudioSource> {
        Q_DECLARE_PUBLIC(MixerAudioSource)
    };
}

#endif // TALCS_MIXERAUDIOSOURCE_P_H
