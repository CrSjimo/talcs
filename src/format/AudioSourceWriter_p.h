#ifndef TALCS_TRANSPORTAUDIOSOURCEWRITER_P_H
#define TALCS_TRANSPORTAUDIOSOURCEWRITER_P_H

#include "AudioSourceWriter.h"

#include "core/buffer/InterleavedAudioDataWrapper.h"
#include "core/base/AudioSourceProcessorBase_p.h"

namespace talcs {
    class AudioSourceWriterPrivate: public AudioSourceProcessorBasePrivate {
    public:
        InterleavedAudioDataWrapper *buf = nullptr;
        AudioFormatIO *outFile;
    };
}

#endif // TALCS_TRANSPORTAUDIOSOURCEWRITER_P_H
