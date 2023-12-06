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

#include "AudioDataWrapper.h"
#include "AudioDataWrapper_p.h"

namespace talcs {

    /**
     * @class AudioDataWrapper
     * @brief A Wrapper that points to a set of pre-allocated arrays of sample data
     *
     * This class is useful when processing audio data stored planarly in the memory.
     * @see InterLeavedAudioDataWrapper
     */

    /**
     * Creates a wrapper that points to a set of pre-allocated arrays of sample data.
     * @param data          a two-dimensional float array contains sample data
     * @param channelCount  the number of channels, indicating the number the first dimension that will be used
     * @param sampleCount   the number of samples, indicating the number of the second dimension that will be used
     * @param startPos      the optional start position of samples in each channel, indicating the starting index of the
     * second dimension
     */
    AudioDataWrapper::AudioDataWrapper(float *const *data, int channelCount, qint64 sampleCount, qint64 startPos)
        : d(new AudioDataWrapperPrivate) {
        d->data = data;
        d->channelCount = channelCount;
        d->sampleCount = sampleCount;
        d->startPos = startPos;
    }

    AudioDataWrapper::~AudioDataWrapper() = default;

    float &AudioDataWrapper::sampleAt(int channel, qint64 pos) {
        return d->data[channel][d->startPos + pos];
    }

    float AudioDataWrapper::constSampleAt(int channel, qint64 pos) const {
        return d->data[channel][d->startPos + pos];
    }

    int AudioDataWrapper::channelCount() const {
        return d->channelCount;
    }

    qint64 AudioDataWrapper::sampleCount() const {
        return d->sampleCount;
    }

    /**
     * Gets the data pointer that a specified channel points to.
     */
    float *AudioDataWrapper::data(int channel) const {
        return d->data[channel] + d->startPos;
    }

    /**
     * Resets the data. This function behaviors like the constructor, but it does that dynamically.
     *
     * @see AudioDataWrapper()
     */
    void AudioDataWrapper::reset(float *const *data, int channelCount, qint64 sampleCount, qint64 startPos) {
        d->data = data;
        d->channelCount = channelCount;
        d->sampleCount = sampleCount;
        d->startPos = startPos;
    }

    float *AudioDataWrapper::writePointerTo(int channel, qint64 startPos) {
        return d->data[channel] + startPos;
    }

    /**
     * This is an overriden function.
     * @return @c true always
     */
    bool AudioDataWrapper::isContinuous() const {
        return true;
    }

    const float *AudioDataWrapper::readPointerTo(int channel, qint64 startPos) const {
        return d->data[channel] + startPos;
    }

    AudioDataWrapper::AudioDataWrapper(const AudioDataWrapper &other) : d(new AudioDataWrapperPrivate(*other.d.data())) {

    }

    AudioDataWrapper::AudioDataWrapper(AudioDataWrapper &&other) : d(other.d.take()) {
    }

    AudioDataWrapper &AudioDataWrapper::operator=(const AudioDataWrapper &other) {
        d.reset(new AudioDataWrapperPrivate(*other.d.data()));
        return *this;
    }

    AudioDataWrapper &AudioDataWrapper::operator=(AudioDataWrapper &&other) {
        d.reset(other.d.take());
        return *this;
    }

    bool AudioDataWrapper::isDuplicatable() const {
        return true;
    }

    DuplicatableObject *AudioDataWrapper::duplicate() const {
        return new AudioDataWrapper(*this);
    }

}
