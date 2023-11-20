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
        : d(new InterleavedAudioDataWrapperPrivate) {
        d->data = data;
        d->channelCount = channelCount;
        d->sampleCount = sampleCount;
    }

    InterleavedAudioDataWrapper::~InterleavedAudioDataWrapper() = default;

    float &InterleavedAudioDataWrapper::sampleAt(int channel, qint64 pos) {
        return d->data[pos * d->channelCount + channel];
    }

    float InterleavedAudioDataWrapper::constSampleAt(int channel, qint64 pos) const {
        return d->data[pos * d->channelCount + channel];
    }

    int InterleavedAudioDataWrapper::channelCount() const {
        return d->channelCount;
    }

    qint64 InterleavedAudioDataWrapper::sampleCount() const {
        return d->sampleCount;
    }

    /**
     * Returns the array that this wrapper points to.
     */
    float *InterleavedAudioDataWrapper::data() const {
        return d->data;
    }

    /**
     * Resets the data. This function behaviors like the constructor, but it does that dynamically.
     * @see InterleavedAudioDataWrapper()
     */
    void InterleavedAudioDataWrapper::reset(float *data, int channelCount, qint64 sampleCount) {
        d->data = data;
        d->channelCount = channelCount;
        d->sampleCount = sampleCount;
    }

    float *InterleavedAudioDataWrapper::writePointerTo(int channel, qint64 startPos) {
        if (d->channelCount == 1 && channel == 0)
            return d->data + startPos;
        return nullptr;
    }

    /**
     * This is an overriden function.
     * @return @c false, unless the number of channel is one.
     */
    bool InterleavedAudioDataWrapper::isContinuous() const {
        return d->channelCount == 1;
    }

    const float *InterleavedAudioDataWrapper::readPointerTo(int channel, qint64 startPos) const {
        if (d->channelCount == 1 && channel == 0)
            return d->data + startPos;
        return nullptr;
    }

    InterleavedAudioDataWrapper::InterleavedAudioDataWrapper(const InterleavedAudioDataWrapper &other) : d(new InterleavedAudioDataWrapperPrivate(*other.d.data())) {
    }

    InterleavedAudioDataWrapper::InterleavedAudioDataWrapper(InterleavedAudioDataWrapper &&other) : d(other.d.take()) {
    }

    InterleavedAudioDataWrapper &InterleavedAudioDataWrapper::operator=(const InterleavedAudioDataWrapper &other) {
        d.reset(new InterleavedAudioDataWrapperPrivate(*other.d.data()));
        return *this;
    }

    InterleavedAudioDataWrapper &InterleavedAudioDataWrapper::operator=(InterleavedAudioDataWrapper &&other) {
        d.reset(other.d.take());
        return *this;
    }

    bool InterleavedAudioDataWrapper::isDuplicatable() const {
        return true;
    }

    DuplicatableObject *InterleavedAudioDataWrapper::duplicate() const {
        return new InterleavedAudioDataWrapper(*this);
    }

}
