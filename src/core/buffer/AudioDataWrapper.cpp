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
        : AudioDataWrapper(*new AudioDataWrapperPrivate) {
        Q_D(AudioDataWrapper);
        d->data = data;
        d->channelCount = channelCount;
        d->sampleCount = sampleCount;
        d->startPos = startPos;
    }

    AudioDataWrapper::~AudioDataWrapper() = default;

    float &AudioDataWrapper::sampleAt(int channel, qint64 pos) {
        Q_D(AudioDataWrapper);
        return d->data[channel][d->startPos + pos];
    }

    float AudioDataWrapper::constSampleAt(int channel, qint64 pos) const {
        Q_D(const AudioDataWrapper);
        return d->data[channel][d->startPos + pos];
    }

    int AudioDataWrapper::channelCount() const {
        Q_D(const AudioDataWrapper);
        return d->channelCount;
    }

    qint64 AudioDataWrapper::sampleCount() const {
        Q_D(const AudioDataWrapper);
        return d->sampleCount;
    }

    /**
     * Gets the data pointer that a specified channel points to.
     */
    float *AudioDataWrapper::data(int channel) const {
        Q_D(const AudioDataWrapper);
        return d->data[channel] + d->startPos;
    }

    /**
     * Resets the data. This function behaviors like the constructor, but it does that dynamically.
     *
     * @see AudioDataWrapper()
     */
    void AudioDataWrapper::reset(float *const *data, int channelCount, qint64 sampleCount, qint64 startPos) {
        Q_D(AudioDataWrapper);
        d->data = data;
        d->channelCount = channelCount;
        d->sampleCount = sampleCount;
        d->startPos = startPos;
    }

    AudioDataWrapper::AudioDataWrapper(AudioDataWrapperPrivate &d) : d_ptr(&d) {
        d.q_ptr = this;
    }

    float *AudioDataWrapper::writePointerTo(int channel, qint64 startPos) {
        Q_D(AudioDataWrapper);
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
        Q_D(const AudioDataWrapper);
        return d->data[channel] + startPos;
    }
    
}
