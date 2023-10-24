#ifndef TALCS_MIXERAUDIOSOURCE_P_H
#define TALCS_MIXERAUDIOSOURCE_P_H

#include <QMutex>
#include <QObject>

#include <TalcsCore/MixerAudioSource.h>
#include <TalcsCore/private/AudioSource_p.h>
#include <TalcsCore/private/IMixer_p.h>
#include <TalcsCore/AudioBuffer.h>

namespace talcs {
    class MixerAudioSourcePrivate : public AudioSourcePrivate, public IMixerPrivate<AudioSource> {
        Q_DECLARE_PUBLIC(MixerAudioSource)
    };
}

#endif // TALCS_MIXERAUDIOSOURCE_P_H
