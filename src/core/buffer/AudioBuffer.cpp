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

#include "AudioBuffer.h"

namespace talcs {

    /**
     * @class AudioBuffer
     * @brief A container of audio sample data
     *
     * AudioBuffer use a two-dimensional QVector to store multi-channel audio internally and encapsulates some
     * interfaces to access the internal vector.
     */

    /**
     * Default constructor. Creates an empty buffer with zero channel and zero sample initially.
     */
    AudioBuffer::AudioBuffer() = default;

    /**
     * Creates an empty buffer with specified number of channels and samples.
     */
    AudioBuffer::AudioBuffer(int channelCount, qint64 sampleCount) {
        resize(channelCount, sampleCount);
    }

    void AudioBuffer::setSample(int channel, qint64 pos, float value) {
        m_buffer[channel][pos] = value;
    }
    float AudioBuffer::sample(int channel, qint64 pos) const {
        return m_buffer.at(channel).at(pos);
    }
    int AudioBuffer::channelCount() const {
        return m_buffer.size();
    }
    qint64 AudioBuffer::sampleCount() const {
        return m_buffer.empty() ? 0 : m_buffer[0].size();
    }

    /**
     * Changes the size of this buffer.
     *
     * For both expanding and contracting, this will keep existing data. New samples will be set to zero after
     * expanding.
     *
     * @param newChannelCount   the optional new number of channels
     * @param newSampleCount    the optional new number of samples
     */
    void AudioBuffer::resize(int newChannelCount, qint64 newSampleCount) {
        if (newChannelCount != -1) {
            m_buffer.resize(newChannelCount);
            if (!m_buffer.empty()) {
                for (auto &vec : m_buffer) {
                    vec.resize(m_buffer[0].size());
                }
            }
        }
        if (newSampleCount != -1) {
            for (auto &vec : m_buffer) {
                vec.resize(newSampleCount);
            }
        }
    }

    /**
     * Gets the pointer to the data of a specified channel for writing. This function behaviors the same as @c
     * writePointerTo(channel,0).
     *
     * Note that only until this buffer is resized does the pointer remains valid.
     * @see resize(), writePointerTo()
     */
    float *AudioBuffer::data(int channel) {
        return m_buffer[channel].data();
    }

    /**
     * Gets the pointer to the data of a specified channel for reading. This function behaviors the same as @c
     * readPointerTo(channel,0).
     *
     * Note that only until this buffer is resized does the pointer remains valid.
     * @see resize(), readPointerTo()
     */
    float const *AudioBuffer::constData(int channel) const {
        return m_buffer.at(channel).data();
    }

    template <typename T>
    static inline QVector<T> vectorSlice(const QVector<T> &l, qint64 s, qint64 t = -1) {
        return QVector<T>(l.cbegin() + s, t == -1 ? l.cend() : l.cbegin() + s + t);
    }

    /**
     * Creates a sub-buffer that contains samples of ranges within specified channels.
     * @param startChannelIndex the index of channel where the section starts at
     * @param startSampleCount  the position of sample where the section starts at
     * @param channelSize       the optional number of channels in the section (all channels by default)
     * @param length            the optional number of samples in the section (all samples by default)
     * @return the new AudioBuffer
     */
    AudioBuffer AudioBuffer::slice(int startChannelIndex, qint64 startSampleCount, int channelSize,
                                   qint64 length) const {
        auto b = vectorSlice(m_buffer, startChannelIndex, channelSize);
        for (auto &vec : b) {
            vec = vectorSlice(vec, startSampleCount, length);
        }
        AudioBuffer newBuf;
        newBuf.m_buffer = std::move(b);
        return newBuf;
    }

    /**
     * Creates an AudioBuffer containing the data from a specified source, and with the same numbers of channels and
     * samples as it.
     */
    AudioBuffer AudioBuffer::from(const IAudioSampleProvider &src) {
        auto channelCount = src.channelCount();
        auto sampleCount = src.sampleCount();
        AudioBuffer buf(channelCount, sampleCount);
        buf.setSampleRange(src);
        return buf;
    }

    float *AudioBuffer::writePointerTo(int channel, qint64 startPos) {
        return m_buffer[channel].data() + startPos;
    }

    /**
     * This is an overriden function.
     * @return @c true always
     */
    bool AudioBuffer::isContinuous() const {
        return true;
    }

    const float *AudioBuffer::readPointerTo(int channel, qint64 startPos) const {
        return m_buffer[channel].constData() + startPos;
    }

}
