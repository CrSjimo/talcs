#ifndef TALCS_MEMORYAUDIOSOURCE_P_H
#define TALCS_MEMORYAUDIOSOURCE_P_H

#include "MemoryAudioSource.h"
#include "PositionableAudioSource_p.h"

#include <QMutex>

namespace talcs {
    class MemoryAudioSourcePrivate : public PositionableAudioSourcePrivate {
        Q_DECLARE_PUBLIC(MemoryAudioSource)
    public:
        IAudioSampleProvider *buffer;
        QMutex mutex;
        bool takeOwnership;
    };
}

#endif // TALCS_MEMORYAUDIOSOURCE_P_H
