#ifndef TALCS_INTERLEAVEDAUDIODATAWRAPPER_P_H
#define TALCS_INTERLEAVEDAUDIODATAWRAPPER_P_H

#include <TalcsCore/InterleavedAudioDataWrapper.h>

namespace talcs {

    class InterleavedAudioDataWrapperPrivate {
    public:
        float *data;
        int channelCount;
        qint64 sampleCount;
    };
    
}

#endif // TALCS_INTERLEAVEDAUDIODATAWRAPPER_P_H
