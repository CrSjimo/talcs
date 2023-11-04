#include "R8BrainResampler.h"
#include "R8BrainResampler_p.h"

namespace talcs {
    R8BrainResampler::R8BrainResampler(double ratio, qint64 bufferSize, const R8BrainResampler::CallbackFunction &callback) : d(new R8BrainResamplerPrivate) {
        d->ratio = ratio;
        d->bufferSize = bufferSize;
        d->cb = callback;
        if (ratio == 1.0) {
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

    R8BrainResampler::~R8BrainResampler() = default;

    void R8BrainResampler::reset() {
        d->resampler->clear();
        d->outputBuffer.clear();
        d->outputBufferOffset = 0;
        d->processedInputLength = 0;
        d->processedOutputLength = 0;
    }

    void R8BrainResampler::process(float *buffer) {
        // If ratio is 1.0 then just copy.
        if (d->copyOnly)
            return d->cb(buffer, d->bufferSize);

        readTag:

        bool readFlag = true;

        // First, get the latency and processed the first block. The output length is always zero.
        if (!d->processedOutputLength) {
            int initialLength = d->resampler->getInLenBeforeOutPos(d->bufferSize);
            d->cb(d->inputBuffer.data(), initialLength);
            std::copy(d->inputBuffer.cbegin(), d->inputBuffer.cend(), d->f64InputBuffer.begin());
            double *outputPointer;
            int outputBlockLength = d->resampler->process(d->f64InputBuffer.data(), initialLength, outputPointer);
            std::copy(outputPointer, outputPointer + outputBlockLength, d->outputBuffer.begin() + d->outputBufferOffset);
            d->processedInputLength += initialLength;
            d->processedOutputLength += d->bufferSize;
            d->outputBufferOffset += outputBlockLength;
            if (outputBlockLength != 0)
                readFlag = false;
        }

        // Read input block, covert it to f64, process, and convert the output block into output buffer.
        if (readFlag) {
            int inputBlockLength = d->resampler->getInLenBeforeOutPos(d->processedOutputLength + d->bufferSize) -
                                   d->processedInputLength;
            d->cb(d->inputBuffer.data(), inputBlockLength);
            std::copy(d->inputBuffer.cbegin(), d->inputBuffer.cend(), d->f64InputBuffer.begin());
            double *outputPointer;
            int outputBlockLength = d->resampler->process(d->f64InputBuffer.data(), inputBlockLength, outputPointer);
            std::copy(outputPointer, outputPointer + outputBlockLength,
                      d->outputBuffer.begin() + d->outputBufferOffset);
            d->processedInputLength += inputBlockLength;
            d->processedOutputLength += d->bufferSize;
            d->outputBufferOffset += outputBlockLength;
        }

        if (d->outputBufferOffset >= d->bufferSize) {
            //=================================     =======
            //|--old offset--|--output block--|     |*****|
            //================================= --> =======
            //|-------buffer size-------|*****|           ^
            //=================================           the new offset

            std::copy(d->outputBuffer.cbegin(), d->outputBuffer.cbegin() + d->bufferSize, buffer);
            std::copy(d->outputBuffer.cbegin() + d->bufferSize, d->outputBuffer.cbegin() + d->outputBufferOffset, d->outputBuffer.begin());
            d->outputBufferOffset = d->outputBufferOffset - d->bufferSize;
        } else {
            //===================================     ===================================
            //|--old offset--|--output block--|       |-------------offset------------|
            //=================================== --> ===================================
            //|-----------buffer size-----------|     |-----------buffer size-----------|
            //===================================     ===================================
            // Read the input again and fill the output buffer from the current offset.
            goto readTag;
        }
    }
} // talcs