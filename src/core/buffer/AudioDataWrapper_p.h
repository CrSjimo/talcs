#ifndef TALCS_AUDIODATAWRAPPER_P_H
#define TALCS_AUDIODATAWRAPPER_P_H

#include "AudioDataWrapper.h"

namespace talcs {
    class AudioDataWrapperPrivate {
        Q_DECLARE_PUBLIC(AudioDataWrapper)
    public:
        AudioDataWrapper *q_ptr;
        float *const *data;
        int channelCount;
        qint64 sampleCount;
        qint64 startPos;
    };
}

#endif // TALCS_AUDIODATAWRAPPER_P_H
