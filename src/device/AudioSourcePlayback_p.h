#ifndef TALCS_AUDIOSOURCEPLAYBACK_P_H
#define TALCS_AUDIOSOURCEPLAYBACK_P_H

#include "AudioSourcePlayback.h"

namespace talcs {
    class AudioSourcePlaybackPrivate {
        Q_DECLARE_PUBLIC(AudioSourcePlayback)
    public:
        AudioSourcePlayback *q_ptr;
        AudioSource *src;
        bool takeOwnership;
    };
}

#endif // TALCS_AUDIOSOURCEPLAYBACK_P_H
