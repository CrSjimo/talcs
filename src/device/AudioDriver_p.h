#ifndef TALCS_AUDIODRIVER_P_H
#define TALCS_AUDIODRIVER_P_H

#include <TalcsDevice/AudioDriver.h>

namespace talcs {

    class AudioDriverPrivate {
        Q_DECLARE_PUBLIC(AudioDriver)
    public:
        AudioDriver *q_ptr;
        bool isInitialized = false;
    };
    
}

#endif // TALCS_AUDIODRIVER_P_H
