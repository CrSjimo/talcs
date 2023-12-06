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

#include "IAudioSampleContainer.h"

#include <algorithm>
#include <functional>
#include <cassert>
#include <cstring>

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
     * @interface IAudioSampleContainer
     * @brief Base class for object containing audio data for read and write
     */

    /**
     * @fn float &IAudioSampleContainer::sampleAt(int channel, qint64 pos);
     * Gets the reference to the sample at a specified channel and position.
     *
     * Note that if the sample data is stored continuously, performance will be better when using the pointer to write
     * directly.
     * @return a non-const reference for writing.
     * @see isContinuous(), writePointerTo()
     */

    /**
     * Gets the write pointer to a specified position of a specified channel.
     *
     * Note that his function should return \c nullptr if the sample data is not stored continuously.
     * @see isContinuous()
     */
    float *IAudioSampleContainer::writePointerTo(int channel, qint64 startPos) {
        return nullptr;
    }

    /**
     * Copies samples from another object to this one.
     * @param destChannel   the channel of this object to copy samples to
     * @param destStartPos  the start position within destination channel
     * @param length        the number of samples to copy
     * @param src           the source object to read from
     * @param srcChannel    the channel of the source object to copy samples from
     * @param srcStartPos   the start position within source channel
     */
    void IAudioSampleContainer::setSampleRange(int destChannel, qint64 destStartPos, qint64 length,
                                               const IAudioSampleProvider &src, int srcChannel, qint64 srcStartPos) {
        boundCheck(*this, destChannel, destStartPos, length);
        boundCheck(src, srcChannel, srcStartPos, length);
        if (isContinuous() && src.isContinuous()) {
            auto pDest = writePointerTo(destChannel, destStartPos);
            auto pSrc = src.readPointerTo(srcChannel, srcStartPos);
            std::copy(pSrc, pSrc + length, pDest);
        } else {
            for (qint64 i = 0; i < length; i++) {
                sampleAt(destChannel, destStartPos + i) = src.constSampleAt(srcChannel, srcStartPos + i);
            }
        }
    }

    /**
     * @overload
     *
     * Copies all channels and samples from another object to this one.
     *
     * Note that if the number of channels or the number of samples in the source object is less than that in this
     * object or vice versa, only the overlapping part will be processed.
     */
    void IAudioSampleContainer::setSampleRange(const IAudioSampleProvider &src) {
        auto minChannelCount = std::min(channelCount(), src.channelCount());
        auto minSampleCount = std::min(sampleCount(), src.sampleCount());
        for (int i = 0; i < minChannelCount; i++) {
            setSampleRange(i, 0, minSampleCount, src, i, 0);
        }
    }

    /**
     * Adds samples from another object to this one.
     * @param destChannel   the channel of this object to copy samples to
     * @param destStartPos  the start position within destination channel
     * @param length        the number of samples to copy
     * @param src           the source object to read from
     * @param srcChannel    the channel of the source object to copy samples from
     * @param srcStartPos   the start position within source channel
     * @param gain          an optional gain to apply to the source before added
     */
    void IAudioSampleContainer::addSampleRange(int destChannel, qint64 destStartPos, qint64 length,
                                               const IAudioSampleProvider &src, int srcChannel, qint64 srcStartPos,
                                               float gain) {
        boundCheck(*this, destChannel, destStartPos, length);
        boundCheck(src, srcChannel, srcStartPos, length);
        if (isContinuous() && src.isContinuous()) {
            auto pDest = writePointerTo(destChannel, destStartPos);
            auto pSrc = src.readPointerTo(srcChannel, srcStartPos);
            std::transform(pDest, pDest + length, pSrc, pDest, std::plus<>());
        } else {
            for (qint64 i = 0; i < length; i++) {
                sampleAt(destChannel, destStartPos + i) += src.constSampleAt(srcChannel, srcStartPos + i) * gain;
            }
        }
    }

    /**
     * @overload
     *
     * Adds all channels and samples from another object to this one.
     *
     * Note that if the number of channels or the number of samples in the source object is less than that in this
     * object or vice versa, only the overlapping part will be processed.
     */
    void IAudioSampleContainer::addSampleRange(const IAudioSampleProvider &src, float gain) {
        auto minChannelCount = std::min(channelCount(), src.channelCount());
        auto minSampleCount = std::min(sampleCount(), src.sampleCount());
        for (int i = 0; i < minChannelCount; i++) {
            addSampleRange(i, 0, minSampleCount, src, i, 0, gain);
        }
    }

    /**
     * Applies gain to samples within a range of a specified channel.
     * @param destChannel   the channel to apply gain to
     * @param destStartPos  the start position within destination channel
     * @param length        the number of samples to apply gain to
     * @param gain          the gain to be applied
     */
    void IAudioSampleContainer::gainSampleRange(int destChannel, qint64 destStartPos, qint64 length, float gain) {
        boundCheck(*this, destChannel, destStartPos, length);
        if (isContinuous()) {
            auto p = writePointerTo(destChannel, destStartPos);
            std::transform(p, p + length, p, [gain](float num) { return num * gain; });
        } else {
            for (qint64 i = 0; i < length; i++) {
                sampleAt(destChannel, destStartPos + i) *= gain;
            }
        }
    }

    /**
     * @overload
     *
     * Applies gain to all samples within a specified channel.
     */
    void IAudioSampleContainer::gainSampleRange(int destChannel, float gain) {
        gainSampleRange(destChannel, 0, sampleCount(), gain);
    }

    /**
     * @overload
     *
     * Applies gain to all samples in all channels.
     */
    void IAudioSampleContainer::gainSampleRange(float gain) {
        auto destChannelCount = channelCount();
        auto destSampleCount = sampleCount();
        for (int i = 0; i < destChannelCount; i++) {
            gainSampleRange(i, 0, destSampleCount, gain);
        }
    }

    /**
     * Sets samples within a range of a specified channel to zero.
     * @param destChannel   the channel to clear
     * @param destStartPos  the start position within destination channel
     * @param length        the number of samples to clear
     */
    void IAudioSampleContainer::clear(int destChannel, qint64 destStartPos, qint64 length) {
        boundCheck(*this, destChannel, destStartPos, length);
        if (isContinuous()) {
            memset(writePointerTo(destChannel, destStartPos), 0, length * sizeof(float));
        } else {
            for (qint64 i = 0; i < length; i++) {
                sampleAt(destChannel, destStartPos + i) = 0;
            }
        }
    }

    /**
     * @overload
     *
     * Sets all samples within a specified channel to zero.
     */
    void IAudioSampleContainer::clear(int destChannel) {
        clear(destChannel, 0, sampleCount());
    }

    /**
     * @overload
     *
     * Sets all samples in all channels to zero.
     */
    void IAudioSampleContainer::clear() {
        auto destChannelCount = channelCount();
        auto destSampleCount = sampleCount();
        for (int i = 0; i < destChannelCount; i++) {
            clear(i, 0, destSampleCount);
        }
    }
    
}
