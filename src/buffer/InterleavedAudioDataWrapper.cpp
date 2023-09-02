#include "InterleavedAudioDataWrapper.h"
#include "InterleavedAudioDataWrapper_p.h"
namespace talcs {
    /**
     * @class InterleavedAudioDataWrapper
     * @brief A Wrapper that points to a pre-allocated array of interleaved audio sample data.
     * @see AudioDataWrapper
     */

    /**
     * Creates a wrapper that points to a set of pre-allocated arrays of sample data.
     * @param data          a float array contains sample data
     * @param channelCount  the number of channels
     * @param sampleCount   the number of samples
     */
    InterleavedAudioDataWrapper::InterleavedAudioDataWrapper(float *data, int channelCount, qint64 sampleCount)
        : InterleavedAudioDataWrapper(*new InterleavedAudioDataWrapperPrivate) {
        Q_D(InterleavedAudioDataWrapper);
        d->data = data;
        d->channelCount = channelCount;
        d->sampleCount = sampleCount;
    }

    InterleavedAudioDataWrapper::~InterleavedAudioDataWrapper() = default;

    float &InterleavedAudioDataWrapper::sampleAt(int channel, qint64 pos) {
        Q_D(InterleavedAudioDataWrapper);
        return d->data[pos * d->channelCount + channel];
    }

    float InterleavedAudioDataWrapper::constSampleAt(int channel, qint64 pos) const {
        Q_D(const InterleavedAudioDataWrapper);
        return d->data[pos * d->channelCount + channel];
    }

    int InterleavedAudioDataWrapper::channelCount() const {
        Q_D(const InterleavedAudioDataWrapper);
        return d->channelCount;
    }

    qint64 InterleavedAudioDataWrapper::sampleCount() const {
        Q_D(const InterleavedAudioDataWrapper);
        return d->sampleCount;
    }

    /**
     * Returns the array that this wrapper points to.
     */
    float *InterleavedAudioDataWrapper::data() const {
        Q_D(const InterleavedAudioDataWrapper);
        return d->data;
    }

    /**
     * Resets the data. This function behaviors like the constructor, but it does that dynamically.
     * @see InterleavedAudioDataWrapper()
     */
    void InterleavedAudioDataWrapper::reset(float *data, int channelCount, qint64 sampleCount) {
        Q_D(InterleavedAudioDataWrapper);
        d->data = data;
        d->channelCount = channelCount;
        d->sampleCount = sampleCount;
    }
    InterleavedAudioDataWrapper::InterleavedAudioDataWrapper(InterleavedAudioDataWrapperPrivate & d) : d_ptr(&d) {
        d.q_ptr = this;
    }

    float *InterleavedAudioDataWrapper::writePointerTo(int channel, qint64 startPos) {
        Q_D(InterleavedAudioDataWrapper);
        if (d->channelCount == 1 && channel == 0)
            return d->data + startPos;
        return nullptr;
    }

    /**
     * This is an overriden function.
     * @return @c false, unless the number of channel is one.
     */
    bool InterleavedAudioDataWrapper::isContinuous() const {
        Q_D(const InterleavedAudioDataWrapper);
        return d->channelCount == 1;
    }

    const float *InterleavedAudioDataWrapper::readPointerTo(int channel, qint64 startPos) const {
        Q_D(const InterleavedAudioDataWrapper);
        if (d->channelCount == 1 && channel == 0)
            return d->data + startPos;
        return nullptr;
    }
}
