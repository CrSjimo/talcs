#ifndef AUDIOSOURCEPLAYBACK_P_H
#define AUDIOSOURCEPLAYBACK_P_H

#include <TalcsDevice/AudioSourcePlayback.h>

namespace talcs {

    class AudioSourcePlaybackPrivate {
        Q_DECLARE_PUBLIC(AudioSourcePlayback)
    public:
        AudioSourcePlayback *q_ptr;
        AudioSource *src;
        bool takeOwnership;
    };
    
}

#endif // AUDIOSOURCEPLAYBACK_P_H
