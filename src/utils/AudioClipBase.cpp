#include "AudioClipBase.h"

namespace talcs {
    /**
     * @struct AudioClipBase
     * @brief Generic struct for audio clips
     * @var AudioClipBase::position
     * The position where the start of clip locates in the clip series. A value of -1 represent the
     * object is null.
     *
     * @var AudioClipBase::content
     * The content of audio clip
     *
     * @var AudioClipBase::startPos
     * The start of range within the content of audio clip
     *
     * @var AudioClipBase::length
     * The length of range within the content of audio clip
     */

    /**
     * @fn bool AudioClipBase::operator<(const AudioClipBase<T> &other) const
     * Compare two audio clips by their positions.
     *
     * This is used for sorting audio clips in an audio clip series.
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

    /**
     * @fn typename std::set<AudioClipBase<T>>::const_iterator AudioClipSeriesBase::findClipIt(qint64 pos) const
     * Gets the iterator that points to the clip at a specified position.
     *
     * This is useful for derived class to implement their functions.
     */
}
