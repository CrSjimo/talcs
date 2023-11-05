#include "R8BrainMultichannelResampler.h"
#include "R8BrainMultichannelResampler_p.h"
#include "TalcsCore/AudioDataWrapper.h"

#include <TalcsCore/AudioSource.h>

namespace talcs {
    R8BrainMultichannelResampler::R8BrainMultichannelResampler(double ratio, qint64 bufferSize, int channelCount) : d(new R8BrainMultichannelResamplerPrivate) {
        Q_ASSERT(channelCount > 0);
        d->channelCount = channelCount;
        d->inputBuffer.resize(channelCount, 1);
        for (int i = 0; i < channelCount; i++) {
            d->resamplerOfChannel.push_back(new ChannelResampler(ratio, bufferSize, this, i));
        }
        d->tmpBuf = new float[bufferSize];
    }

    R8BrainMultichannelResampler::~R8BrainMultichannelResampler() {
        for (int i = 0; i < d->channelCount; i++) {
            delete d->resamplerOfChannel[i];
        }
        delete[] d->tmpBuf;
    }

    void R8BrainMultichannelResampler::reset() {
        for (auto resampler: d->resamplerOfChannel) {
            resampler->reset();
        }
    }

    void ChannelResampler::read(float *inputBlock, qint64 length) {
        if (ch == 0) {
            mcr->d->inputBuffer.resize(-1, length);
            mcr->read(&mcr->d->inputBuffer);
        }
        auto *p = mcr->d->inputBuffer.constData(ch);
        std::copy(p, p + length, inputBlock);
    }

    void R8BrainMultichannelResampler::process(const AudioSourceReadData &readData) {
        if (readData.buffer->isContinuous()) {
            for (int i = 0; i < d->channelCount; i++) {
                if (i < readData.buffer->channelCount())
                    d->resamplerOfChannel[i]->process(readData.buffer->writePointerTo(i, readData.startPos));
                else
                    d->resamplerOfChannel[i]->process(d->tmpBuf);
            }
        } else {
            AudioDataWrapper pSrc(&d->tmpBuf, 1, bufferSize());
            for (int i = 0; i < d->channelCount; i++) {
                d->resamplerOfChannel[i]->process(d->tmpBuf);
                if (i < readData.buffer->channelCount())
                    readData.buffer->setSampleRange(i, readData.startPos, readData.length, pSrc, 0, 0);
            }
        }
    }

    double R8BrainMultichannelResampler::ratio() const {
        return d->resamplerOfChannel[0]->ratio();
    }

    qint64 R8BrainMultichannelResampler::bufferSize() const {
        return d->resamplerOfChannel[0]->bufferSize();
    }
} // talcs