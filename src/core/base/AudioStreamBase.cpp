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

#include "AudioStreamBase.h"

namespace talcs {

    /**
     * @class AudioStreamBase
     * @brief Base class for audio streams
     */

    /**
     * Opens the stream.
     *
     * All reimplemented functions should call this function.
     *
     * @note The function might be called when the stream is already open. In this case, the stream
     * should be closed first and then opened.
     * @param bufferSize the size of each block to process
     * @param sampleRate the sample rate of audio
     * @return @c true if success
     */
    bool AudioStreamBase::open(qint64 bufferSize, double sampleRate) {
        m_bufferSize = bufferSize;
        m_sampleRate = sampleRate;
        m_isOpened = true;
        return true;
    }

    /**
     * Gets whether the stream is open.
     */
    bool AudioStreamBase::isOpen() const {
        return m_isOpened;
    }

    /**
     * Closes the stream.
     *
     * All reimplemented functions should call this function.
     *
     * @note The function might be called when the stream is close. In this case, derived classes should handle it
     * properly.
     */
    void AudioStreamBase::close() {
        m_bufferSize = 0;
        m_sampleRate = 0;
        m_isOpened = false;
    }

    /**
     * Gets the buffer size. If not open, 0 will be returned.
     * @see open()
     */
    qint64 AudioStreamBase::bufferSize() const {
        return m_bufferSize;
    }

    /**
     * Gets the sample rate. If not open, 0.0 will be returned.
     * @see open()
     */
    double AudioStreamBase::sampleRate() const {
        return m_sampleRate;
    }
    
}
