#ifndef TALCS_TRANSPORTAUDIOSOURCEWRITER_P_H
#define TALCS_TRANSPORTAUDIOSOURCEWRITER_P_H

#include <TalcsCore/InterleavedAudioDataWrapper.h>
#include <TalcsCore/private/AudioSourceProcessorBase_p.h>

#include <TalcsFormat/AudioSourceWriter.h>

namespace talcs {

    class AudioSourceWriterPrivate: public AudioSourceProcessorBasePrivate {
    public:
        InterleavedAudioDataWrapper *buf = nullptr;
        AudioFormatIO *outFile;
    };
    
}

#endif // TALCS_TRANSPORTAUDIOSOURCEWRITER_P_H
