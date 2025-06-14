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

#include "IAudioSampleProvider.h"

#include <algorithm>
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
     * @interface IAudioSampleProvider
     * @brief Base class for object containing audio data for read
     */

    /**
     * @fn float IAudioSampleProvider::sample(int channel, qint64 pos) const
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
                m = qMax(m, std::abs(sample(channel, startPos + i)));
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
     * Gets the minimum and maximum sample values within a range of a specified channel.
     */
    QPair<float, float> IAudioSampleProvider::findMinMax(int channel, qint64 startPos, qint64 length) const {
        boundCheck(*this, channel, startPos, length);
        if (!isContinuous()) {
            auto m = qMakePair(std::numeric_limits<float>::max(), std::numeric_limits<float>::lowest());
            for (qint64 i = 0; i < length; i++) {
                m.first = qMin(m.first, sample(channel, startPos + i));
                m.second = qMax(m.second, sample(channel, startPos + i));
            }
            return m;
        } else {
            auto p = readPointerTo(channel, startPos);
            auto ret = std::minmax_element(p, p + length);
            return qMakePair(*ret.first, *ret.second);
        }
    }

    /**
     * @overload
     *
     * Gets the minimum and maximum sample values within a specified channel.
     */
    QPair<float, float> IAudioSampleProvider::findMinMax(int channel) const {
        return findMinMax(channel, 0, sampleCount());
    }

    /**
     * Calculates the root mean squared sample value within a range of a specified channel.
     */
    float IAudioSampleProvider::rms(int channel, qint64 startPos, qint64 length) const {
        boundCheck(*this, channel, startPos, length);
        if (!isContinuous()) {
            float s = 0;
            for (qint64 i = 0; i < length; i++) {
                auto v = sample(channel, startPos + i);
                s += v * v;
            }
            return std::sqrt(s / static_cast<float>(length));
        } else {
            auto p = readPointerTo(channel, startPos);
            return std::sqrt(
                std::accumulate(p, p + length, 0.0f, [](float sum, float num) { return sum + num * num; }) / static_cast<float>(length));
        }
    }

    /**
     * Calculates the root mean squared sample value within a specified channel.
     */
    float IAudioSampleProvider::rms(int channel) const {
        return rms(channel, 0, sampleCount());
    }
    
}