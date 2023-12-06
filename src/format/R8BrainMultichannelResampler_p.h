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

#ifndef TALCS_R8BRAINMULTICHANNELRESAMPLER_P_H
#define TALCS_R8BRAINMULTICHANNELRESAMPLER_P_H

#include "R8BrainMultichannelResampler.h"

#include "R8BrainResampler.h"
#include <TalcsCore/AudioBuffer.h>

namespace talcs {

    class ChannelResampler;

    class R8BrainMultichannelResamplerPrivate {
    public:
        int channelCount;
        std::vector<ChannelResampler *> resamplerOfChannel;
        AudioBuffer inputBuffer;
        float *tmpBuf = nullptr;
    };

    class ChannelResampler : public R8BrainResampler {
    public:
        ChannelResampler(double ratio, qint64 bufferSize, R8BrainMultichannelResampler *mcr, int ch) : R8BrainResampler(ratio, bufferSize), mcr(mcr), ch(ch) {}
        void read(float *inputBlock, qint64 length) override;
        R8BrainMultichannelResampler *mcr;
        int ch;
    };
}

#endif //TALCS_R8BRAINMULTICHANNELRESAMPLER_P_H
