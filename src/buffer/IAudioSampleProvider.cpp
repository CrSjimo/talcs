#include "IAudioSampleProvider.h"

#include <cassert>
#include <cmath>
#include <numeric>

namespace talcs {
    /**
     * @internal
     * Prevent the range accessed from exceeding the boundary.
     */
    static inline void boundCheck(const IAudioSampleProvider &iAudioStorage, int channel, qint64 startPos,
                                  qint64 length) {
        assert(channel >= 0 && channel < iAudioStorage.channelCount());
        assert(startPos >= 0 && startPos <= iAudioStorage.sampleCount());
        assert(startPos + length >= 0 && startPos + length <= iAudioStorage.sampleCount());
    }

    /**
     * @class IAudioSampleProvider
     * @brief Base class for object containing audio data for read
     */

    /**
     * @fn float IAudioSampleProvider::constSampleAt(int channel, qint64 pos) const
     * Gets the sample at a specified channel and position.
     *
     * Note that if the sample data is stored continuously, performance will be better when using the pointer to read
     * directly.
     * @see isContinuous(), readPointerTo()
     */

    /**
     * Gets whether the sample data is stored continuously or not. If so, the pointer can be used to read data directly.
     * @see readPointerTo()
     */
    bool IAudioSampleProvider::isContinuous() const {
        return false;
    }

    /**
     * Gets the read pointer to a specified position of a specified channel.
     *
     * Note that his function should return @c nullptr if the sample data is not stored continuously.
     * @see isContinuous()
     */
    const float *IAudioSampleProvider::readPointerTo(int channel, qint64 startPos) const {
        return nullptr;
    }

    /**
     * @fn int IAudioSampleProvider::channelCount() const
     * Gets the number of channels.
     */

    /**
     * @fn qint64 IAudioSampleProvider::sampleCount() const
     * Gets the number of samples that every channel contains.
     */

    /**
     * Gets the highest absolute sample value within a range of a specified channel.
     */
    float IAudioSampleProvider::magnitude(int channel, qint64 startPos, qint64 length) const {
        boundCheck(*this, channel, startPos, length);
        if (!isContinuous()) {
            float m = 0;
            for (qint64 i = 0; i < length; i++) {
                m = std::max(m, std::abs(constSampleAt(channel, startPos + i)));
            }
            return m;
        } else {
            auto p = readPointerTo(channel, startPos);
            return std::abs(
                *std::max_element(p, p + length, [](float lhs, float rhs) { return std::abs(lhs) < std::abs(rhs); }));
        }
    }

    /**
     * @overload
     *
     * Gets the highest absolute sample value within a specified channel.
     */
    float IAudioSampleProvider::magnitude(int channel) const {
        return magnitude(channel, 0, sampleCount());
    }

    /**
     * Calculates the root mean squared sample value within a range of a specified channel.
     */
    float IAudioSampleProvider::rms(int channel, qint64 startPos, qint64 length) const {
        boundCheck(*this, channel, startPos, length);
        if (!isContinuous()) {
            float s = 0;
            for (qint64 i = 0; i < length; i++) {
                auto sample = constSampleAt(channel, startPos + i);
                s += sample * sample;
            }
            return std::sqrt(s / length);
        } else {
            auto p = readPointerTo(channel, startPos);
            return std::sqrt(
                std::accumulate(p, p + length, 0.0f, [](float sum, float num) { return sum + num * num; }) / length);
        }
    }

    /**
     * Calculates the root mean squared sample value within a specified channel.
     */
    float IAudioSampleProvider::rms(int channel) const {
        return rms(channel, 0, sampleCount());
    }
}