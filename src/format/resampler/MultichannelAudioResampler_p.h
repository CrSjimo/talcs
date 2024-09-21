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

#ifndef TALCS_MULTICHANNELAUDIORESAMPLER_P_H
#define TALCS_MULTICHANNELAUDIORESAMPLER_P_H

#include <memory>
#include <vector>

#include "MultichannelAudioResampler.h"
#include "AudioResampler.h"

#include <TalcsCore/AudioBuffer.h>

namespace talcs {

    class ChannelResampler;

    class MultichannelAudioResamplerPrivate {
    public:
        int channelCount;
        std::vector<std::unique_ptr<ChannelResampler>> resamplerOfChannel;
        AudioBuffer inputBuffer;
        std::unique_ptr<float[]> tmpBuf;
    };

    class ChannelResampler : public AudioResampler {
    public:
        ChannelResampler(double ratio, qint64 bufferSize, MultichannelAudioResampler *mcr, int ch) : AudioResampler(ratio, bufferSize), mcr(mcr), ch(ch) {}
        void read(float *inputBlock, qint64 length) override;
        MultichannelAudioResampler *mcr;
        int ch;
    };
}

#endif //TALCS_MULTICHANNELAUDIORESAMPLER_P_H
