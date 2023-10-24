#ifndef TALCS_AUDIOSOURCE_P_H
#define TALCS_AUDIOSOURCE_P_H

#include "AudioSource.h"

namespace talcs {
    class AudioSourcePrivate {
        Q_DECLARE_PUBLIC(AudioSource)
    public:
        AudioSource *q_ptr;
    };
}

#endif // TALCS_AUDIOSOURCE_P_H
