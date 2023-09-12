#ifndef TALCS_TRANSPORTAUDIOSOURCEWRITER_P_H
#define TALCS_TRANSPORTAUDIOSOURCEWRITER_P_H

#include "AudioSourceWriter.h"

#include "buffer/InterleavedAudioDataWrapper.h"
#include "utils/AudioSourceProcessorBase_p.h"

namespace talcs {
    class AudioSourceWriterPrivate: public AudioSourceProcessorBasePrivate {
    public:
        InterleavedAudioDataWrapper *buf = nullptr;
        AudioFormatIO *outFile;
    };
}

#endif // TALCS_TRANSPORTAUDIOSOURCEWRITER_P_H
