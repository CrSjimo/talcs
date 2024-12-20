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

#include "AudioResampler.h"
#include "AudioResampler_p.h"

#include <algorithm>

namespace talcs {

    /**
     * @class AudioResampler
     * @brief An adapter class of [r8b::CDSPResampler](https://www.voxengo.com/public/r8brain-free-src/Documentation/a00114.html)
     * that provides Secret-Rabbit-Code-like APIs.
     */

    /**
     * Constructor.
     * @param ratio destination sample rate / source sample rate
     * @param bufferSize the size of each output block
     */

    AudioResampler::AudioResampler(double ratio, qint64 bufferSize) : d(new AudioResamplerPrivate) {
        Q_ASSERT(ratio > 0.0);
        d->ratio = ratio;
        d->bufferSize = static_cast<int>(bufferSize);
        if (qFuzzyCompare(ratio, 1.0)) {
            d->copyOnly = true;
            return;
        }
        int maxInLen = r8b::CDSPResampler(1, ratio, d->bufferSize).getInLenBeforeOutPos(d->bufferSize);
        d->resampler.reset(new r8b::CDSPResampler(1, ratio, maxInLen));
        int outputBufferSize = r8b::CDSPResampler(1, ratio, std::ceil(d->bufferSize / ratio)).getMaxOutLen(0);
        d->inputBuffer.resize(maxInLen);
        d->f64InputBuffer.resize(maxInLen);
        d->outputBuffer.resize(outputBufferSize * 2);
    }

    /**
     * Destructor.
     */
    AudioResampler::~AudioResampler() = default;

    /**
     * Resets the initial states of the resampler. This function should be called when the source is changed.
     */
    void AudioResampler::reset() {
        if (d->copyOnly)
            return;
        d->resampler->clear();
        std::fill(d->outputBuffer.begin(), d->outputBuffer.end(), 0);
        d->outputBufferOffset = 0;
        d->processedInputLength = 0;
        d->processedOutputLength = 0;
    }

    /**
     * Gets the ratio of the resampler.
     */
    double AudioResampler::ratio() const {
        return d->ratio;
    }

    /**
     * Gets the buffer size of the resampler.
     */
    qint64 AudioResampler::bufferSize() const {
        return d->bufferSize;
    }

    /**
     * Processes the output buffer.
     *
     * This function does not provide functionalities of getting read length, and it should be checked in other ways.
     */
    void AudioResampler::process(float *buffer, qint64 length) {
        // If ratio is 1.0 then just copy.
        if (d->copyOnly)
            return read(buffer, length);

        if (length == 0)
            return;


        for (int i = 0; i < 2; i++) {
            bool readFlag = true;

            // First, get the latency and processed the first block. The output length is always zero.
            if (!d->processedOutputLength) {
                int initialLength = d->resampler->getInLenBeforeOutPos(static_cast<int>(length));
                read(d->inputBuffer.data(), initialLength);
                std::copy_n(d->inputBuffer.cbegin(), initialLength, d->f64InputBuffer.begin());
                double *outputPointer;
                int outputBlockLength = d->resampler->process(d->f64InputBuffer.data(), initialLength, outputPointer);
                std::copy_n(outputPointer, outputBlockLength,
                          d->outputBuffer.begin() + d->outputBufferOffset);
                d->processedInputLength += initialLength;
                d->processedOutputLength += static_cast<int>(length);
                d->outputBufferOffset += outputBlockLength;
                if (outputBlockLength != 0)
                    readFlag = false;
            }

            // Read input block, covert it to f64, process, and convert the output block into output buffer.
            if (readFlag) {
                int inputBlockLength = d->resampler->getInLenBeforeOutPos(d->processedOutputLength + static_cast<int>(length)) -
                                       d->processedInputLength;
                read(d->inputBuffer.data(), inputBlockLength);
                std::copy_n(d->inputBuffer.cbegin(), inputBlockLength,
                          d->f64InputBuffer.begin());
                double *outputPointer;
                int outputBlockLength = d->resampler->process(d->f64InputBuffer.data(), inputBlockLength,
                                                              outputPointer);
                std::copy_n(outputPointer, outputBlockLength,
                          d->outputBuffer.begin() + d->outputBufferOffset);
                d->processedInputLength += inputBlockLength;
                d->processedOutputLength += static_cast<int>(length);
                d->outputBufferOffset += outputBlockLength;
            }

            if (d->outputBufferOffset >= length) {
                //=================================     =======
                //|--old offset--|--output block--|     |*****|
                //================================= --> =======
                //|-------buffer size-------|*****|           ^
                //=================================           the new offset

                std::copy_n(d->outputBuffer.cbegin(), length, buffer);
                std::copy_n(d->outputBuffer.cbegin() + length, d->outputBufferOffset - length,
                          d->outputBuffer.begin());
                d->outputBufferOffset = d->outputBufferOffset - static_cast<int>(length);
                break;
            } else {
                //===================================     ===================================
                //|--old offset--|--output block--|       |-------------offset------------|
                //=================================== --> ===================================
                //|-----------buffer size-----------|     |-----------buffer size-----------|
                //===================================     ===================================
                // Read the input again and fill the output buffer from the current offset.
                continue;
            }
        }
    }

    /**
     * @fn void AudioResampler::read(float *inputBlock, qint64 length)
     * Reads an input block of specified length. If the source is not able to provide
     * a block of the specified length, the function needs to fill the exceeding part with zeros.
     */

} // talcs