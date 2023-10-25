#ifndef TALCS_AUDIOSOURCEPROCESSORBASE_P_H
#define TALCS_AUDIOSOURCEPROCESSORBASE_P_H

#include <TalcsCore/AudioSourceProcessorBase.h>

namespace talcs {

    class AudioSourceProcessorBasePrivate {
        Q_DECLARE_PUBLIC(AudioSourceProcessorBase)
    public:
        AudioSourceProcessorBase *q_ptr;
        AudioSource *src;
        qint64 length;
        QAtomicInteger<bool> stopRequested = false;
        AudioSourceProcessorBase::Status status = AudioSourceProcessorBase::Ready;
    };

}

#endif // TALCS_AUDIOSOURCEPROCESSORBASE_P_H
