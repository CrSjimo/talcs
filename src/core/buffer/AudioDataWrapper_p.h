#ifndef TALCS_AUDIODATAWRAPPER_P_H
#define TALCS_AUDIODATAWRAPPER_P_H

#include <TalcsCore/AudioDataWrapper.h>

namespace talcs {

    class AudioDataWrapperPrivate {
    public:
        float *const *data;
        int channelCount;
        qint64 sampleCount;
        qint64 startPos;
    };
    
}

#endif // TALCS_AUDIODATAWRAPPER_P_H
