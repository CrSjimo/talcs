#ifndef TALCS_TRANSPORTAUDIOSOURCEWRITER_P_H
#define TALCS_TRANSPORTAUDIOSOURCEWRITER_P_H

#include "TransportAudioSourceWriter.h"

#include "source/TransportAudioSource_p.h"

namespace talcs {
    class TransportAudioSourceWriterPrivate {
        Q_DECLARE_PUBLIC(TransportAudioSourceWriter);

    public:
        TransportAudioSourceWriter *q_ptr;
        TransportAudioSource *src;
        TransportAudioSourcePrivate *pSrc;
        AudioFormatIO *outFile;
        qint64 startPos;
        qint64 length;
        QAtomicInteger<bool> stopRequested = false;
    };
}

#endif // TALCS_TRANSPORTAUDIOSOURCEWRITER_P_H
