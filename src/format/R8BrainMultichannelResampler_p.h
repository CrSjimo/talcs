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
