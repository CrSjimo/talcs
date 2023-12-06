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

#include "AudioClipBase.h"

namespace talcs {
    
    /**
     * @struct AudioClipBase
     * @brief Generic struct for audio clips
     */

    /**
     * @fn AudioClipBase::AudioClipBase(qint64 position, T *content, qint64 startPos, qint64 length)
     * Constructor.
     * @param position The position where the start of clip locates in the clip series. A value of -1 represent the
     * object is null.
     * @param content The content of audio clip
     * @param startPos The start of range within the content of audio clip
     * @param length The length of range within the content of audio clip
     */

    /**
     * @fn AudioClipBase::AudioClipBase(qint64 position, qint64 length)
     * Constructor. This is used to create a clip with no content, used for overlapping searching.
     *
     * @overload
     */

    /**
     * @fn AudioClipBase::AudioClipBase()
     * Default constructor. Creates a null clip.
     *
     * @overload
     */

    /**
     * @fn qint64 AudioClipBase::position() const
     * Gets the position where the start of clip locates in the clip series. A value of -1 represent the
     * object is null.
     */

    /**
     * @fn T *AudioClipBase::content() const
     * Gets the content of audio clip.
     */

    /**
     * @fn qint64 AudioClipBase::contentStartPosition() const
     * Gets the start of range within the content of audio clip.
     */

    /**
     * @fn qint64 AudioClipBase::endPosition() const
     * Gets the position where the end of clip locates in the clip series. Equal to position() + length().
     */

    /**
     * @fn qint64 AudioClipBase::length() const
     * Gets the length of audio clip.
     */

    /**
     * @class AudioClipSeriesBase
     * @brief Generic class for audio clip series
     * @var AudioClipSeriesBase::m_clips
     * A set that stores the data of all clips
     */

    /**
     * @fn bool AudioClipSeriesBase::addClip(const AudioClipBase<T> &clip)
     * Add a clip to the series.
     *
     * All reimplemented functions should call this function or add the clip to m_clips set in other ways.
     *
     * @returns @c true if success, @c false if overlaps
     */

    /**
     * @fn AudioClipBase<T> AudioClipSeriesBase::findClipAt(qint64 pos) const
     * Gets the clip at a specified position. If there is no clip at the specified position, a null
     * object will be returned.
     */

    /**
     * @fn QList<AudioClipBase<T>> AudioClipSeriesBase::clips() const
     * Gets all clips.
     */

    /**
     * @fn bool AudioClipSeriesBase::removeClipAt(qint64 pos)
     * Removes a clip from the series.
     * @returns @c true if success, @c false if there is no clip
     */

    /**
     * @fn void AudioClipSeriesBase::clearClips()
     * Removes all clips from the series.
     */

    /**
     * @fn qint64 AudioClipSeriesBase::effectiveLength() const
     * Gets the effective length of the series.
     *
     * This represents the maximum end position among all clips.
     */

}
