#ifndef TALCS_R8BRAINRESAMPLER_P_H
#define TALCS_R8BRAINRESAMPLER_P_H

#include "R8BrainResampler.h"

#include <CDSPResampler.h>

#include <QVector>

namespace talcs {
    class R8BrainResamplerPrivate {
    public:
        bool copyOnly = false;
        double ratio;
        int bufferSize;
        QScopedPointer<r8b::CDSPResampler> resampler;
        QVector<float> inputBuffer;
        QVector<double> f64InputBuffer;
        QVector<float> outputBuffer;
        int outputBufferOffset = 0;
        int processedInputLength = 0;
        int processedOutputLength = 0;
    };
}

#endif //TALCS_R8BRAINRESAMPLER_P_H
