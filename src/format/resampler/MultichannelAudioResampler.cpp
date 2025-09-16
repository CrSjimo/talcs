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

#include "MultichannelAudioResampler.h"
#include "MultichannelAudioResampler_p.h"
#include "TalcsCore/AudioDataWrapper.h"

namespace talcs {
    /**
     * @class MultichannelAudioResampler
     * @brief An adapter class of [r8b::CDSPResampler](https://www.voxengo.com/public/r8brain-free-src/Documentation/a00114.html)
     * that provides Secret-Rabbit-Code-like APIs with the ability to process multi-channel audio.
     */

    /**
     * Constructor.
     * @param ratio destination sample rate / source sample rate
     * @param bufferSize the size of each output block
     * @param channelCount the number of audio channels
     */
    MultichannelAudioResampler::MultichannelAudioResampler(double ratio, qint64 bufferSize, int channelCount) : d(new MultichannelAudioResamplerPrivate) {
        Q_ASSERT(channelCount > 0);
        d->channelCount = channelCount;
        d->inputBuffer.resize(channelCount, 1);
        for (int i = 0; i < channelCount; i++) {
            d->resamplerOfChannel.emplace_back(std::make_unique<ChannelResampler>(ratio, bufferSize, this, i));
        }
        d->tmpBuf.reset(new float[bufferSize]);
    }

    /**
     * Destructor.
     */
    MultichannelAudioResampler::~MultichannelAudioResampler() = default;

    /**
     * @copydoc AudioResampler::reset()
     */
    void MultichannelAudioResampler::reset() {
        for (auto &resampler: d->resamplerOfChannel) {
            resampler->reset();
        }
    }

    void ChannelResampler::read(float *inputBlock, qint64 length) {
        if (ch == 0) {
            mcr->d->inputBuffer.resize(-1, length);
            mcr->read(&mcr->d->inputBuffer);
        }
        auto *p = mcr->d->inputBuffer.constData(ch);
        std::copy_n(p, length, inputBlock);
    }

    /**
     * @copydoc AudioResampler::process()
     */
    void MultichannelAudioResampler::process(const AudioSourceReadData &readData) {
        if (readData.buffer->isContinuous()) {
            for (int i = 0; i < d->channelCount; i++) {
                if (i < readData.buffer->channelCount())
                    d->resamplerOfChannel[i]->process(readData.buffer->writePointerTo(i, readData.startPos), readData.length);
                else
                    d->resamplerOfChannel[i]->process(d->tmpBuf.get(), readData.length);
            }
        } else {
            auto p = d->tmpBuf.get();
            AudioDataWrapper pSrc(&p, 1, bufferSize());
            for (int i = 0; i < d->channelCount; i++) {
                d->resamplerOfChannel[i]->process(d->tmpBuf.get(), readData.length);
                if (i < readData.buffer->channelCount())
                    readData.buffer->setSampleRange(i, readData.startPos, readData.length, pSrc, 0, 0);
            }
        }
    }

    /**
     * @copydoc AudioResampler::ratio()
     */
    double MultichannelAudioResampler::ratio() const {
        return d->resamplerOfChannel[0]->ratio();
    }

    /**
     * @copydoc AudioResampler::bufferSize()
     */
    qint64 MultichannelAudioResampler::bufferSize() const {
        return d->resamplerOfChannel[0]->bufferSize();
    }

    /**
     * Gets the number of channels of the resampler.
     */
    int MultichannelAudioResampler::channelCount() const {
        return d->resamplerOfChannel.size();
    }

    /**
     * @fn void MultichannelAudioResampler::read(const AudioSourceReadData &readData)
     * @copydoc AudioResampler::read()
     */
} // talcs
