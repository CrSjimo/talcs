/******************************************************************************
 * Copyright (c) 2023 CrSjimo                                                 *
 *                                                                            *
 * This file is part of TALCS.                                                *
 *                                                                            *
 * TALCS is free software: you can redistribute it and/or modify it under the *
 * terms of the GNU Lesser General Public License as published by the Free    *
 * Software Foundation, either version 3 of the License, or (at your option)  *
 * any later version.                                                         *
 *                                                                            *
 * TALCS is distributed in the hope that it will be useful, but WITHOUT ANY   *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS  *
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for    *
 * more details.                                                              *
 *                                                                            *
 * You should have received a copy of the GNU Lesser General Public License   *
 * along with TALCS. If not, see <https://www.gnu.org/licenses/>.             *
 ******************************************************************************/

#ifndef TALCS_AUDIORESAMPLER_P_H
#define TALCS_AUDIORESAMPLER_P_H

#include "AudioResampler.h"

#include <QVector>

#include <r8brain-free-src/CDSPResampler.h>

namespace talcs {
    class AudioResamplerPrivate {
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

#endif //TALCS_AUDIORESAMPLER_P_H
