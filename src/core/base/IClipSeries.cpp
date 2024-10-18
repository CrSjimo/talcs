/******************************************************************************
 * Copyright (c) 2024 CrSjimo                                                 *
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

#include "IClipSeries.h"
#include "IClipSeries_p.h"

namespace talcs {
    ClipViewImpl::ClipViewImpl() : d(nullptr), m_content(nullptr) {

    }

    bool ClipViewImpl::isValid() const {
        return m_content && d->clipPositionDict.contains(m_content);
    }

    void *ClipViewImpl::content() const {
        return m_content;
    }

    qint64 ClipViewImpl::startPos() const {
        Q_ASSERT(isValid());
        return d->clipStartPosDict.value(m_content);
    }

    qint64 ClipViewImpl::position() const {
        Q_ASSERT(isValid());
        return d->clipPositionDict.value(m_content);
    }

    qint64 ClipViewImpl::length() const {
        Q_ASSERT(isValid());
        return d->intervalLookup(position(), content()).length();
    }

    ClipViewImpl::ClipViewImpl(const IClipSeriesPrivate *d, void *content) : d(d), m_content(content) {

    }

    ClipViewImpl IClipSeriesPrivate::insertClip(void *content, qint64 position, qint64 startPos, qint64 length) {
        if (clipPositionDict.contains(content))
            return {};
        auto interval = ClipInterval(content, position, length);
        clips.insert(interval);
        clipPositionDict.insert(content, position);
        clipStartPosDict.insert(content, startPos);
        endSet.insert(position + length);
        return ClipViewImpl(this, content);
    }

    void IClipSeriesPrivate::setClipStartPos(const ClipViewImpl &clipViewImpl, qint64 startPos) {
        Q_ASSERT(clipViewImpl.isValid());
        clipStartPosDict[clipViewImpl.content()] = startPos;
    }

    bool IClipSeriesPrivate::setClipRange(const ClipViewImpl &clipViewImpl, qint64 position, qint64 length) {
        auto it = findClipIterator(clipPositionDict.value(clipViewImpl.content()), clipViewImpl.content());
        auto oldPosition = it->interval().position();
        auto oldLength = it->interval().length();
        clips.erase(it);
        auto interval = IClipSeriesPrivate::ClipInterval(clipViewImpl.content(), position, length);
        clips.insert(interval);
        clipPositionDict[clipViewImpl.content()] = position;
        endSet.erase(oldPosition + oldLength);
        endSet.insert(position + length);
        return true;
    }

    ClipViewImpl IClipSeriesPrivate::setClipContent(const ClipViewImpl &clipViewImpl, void *content) {
        if (content == clipViewImpl.content())
            return clipViewImpl;
        if (clipPositionDict.contains(content))
            return {};
        auto position = clipViewImpl.position();
        auto startPos = clipViewImpl.startPos();
        auto length = clipViewImpl.length();
        removeClip(clipViewImpl);
        auto ret = insertClip(content, position, startPos, length);
        Q_ASSERT(!ret.isNull());
        return ret;
    }

    ClipViewImpl IClipSeriesPrivate::findClipByContent(void *content) const {
        auto ret = ClipViewImpl(this, content);
        if (!ret.isValid())
            ret.m_content = nullptr;
        return ret;
    }

    void IClipSeriesPrivate::findClipByPosition(qint64 position, const std::function<bool(const ClipViewImpl &)> &onFind) const {
        clips.overlap_find_all({nullptr, position, 1}, [&](const ClipIntervalTree::const_iterator &it) {
            return onFind(ClipViewImpl(this, it->interval().content()));
        });
    }

    void IClipSeriesPrivate::removeClip(const ClipViewImpl &clipViewImpl) {
        auto pos = clipViewImpl.position();
        auto it = findClipIterator(pos, clipViewImpl.content());
        auto endPos = pos + it->interval().length();
        clips.erase(it);
        clipPositionDict.remove(clipViewImpl.content());
        clipStartPosDict.remove(clipViewImpl.content());
        endSet.erase(endPos);
    }

    void IClipSeriesPrivate::removeAllClips() {
        clips.clear();
        clipPositionDict.clear();
        clipStartPosDict.clear();
        endSet.clear();
    }

    QList<ClipViewImpl> IClipSeriesPrivate::clipViewImplList() const {
        QList<ClipViewImpl> list;
        for (auto p = clips.cbegin(); p != clips.cend(); p++) {
            list.append(ClipViewImpl(this, p->interval().content()));
        }
        return list;
    }

    qint64 IClipSeriesPrivate::effectiveLength() const {
        if (endSet.empty())
            return 0;
        return *endSet.rbegin();
    }

    IClipSeriesPrivate::ClipInterval IClipSeriesPrivate::intervalLookup(qint64 pos, void *content) const {
        return findClipIterator(pos, content)->interval();
    }

    IClipSeriesPrivate::ClipIntervalTree::iterator IClipSeriesPrivate::findClipIterator(qint64 pos, void *content) {
        ClipIntervalTree::iterator it = clips.end();
        clips.overlap_find_all({nullptr, pos, 1}, [&](const ClipIntervalTree::iterator &it_) {
            if (it_.interval().content() == content) {
                it = it_;
                return false;
            }
            return true;
        });
        return it;
    }

    IClipSeriesPrivate::ClipIntervalTree::const_iterator IClipSeriesPrivate::findClipIterator(qint64 pos, void *content) const {
        ClipIntervalTree::const_iterator it = clips.end();
        clips.overlap_find_all({nullptr, pos, 1}, [&](const ClipIntervalTree::const_iterator &it_) {
            if (it_.interval().content() == content) {
                it = it_;
                return false;
            }
            return true;
        });
        return it;
    }

    /**
     * @class IClipSeries
     * @brief Generic class for clip series.
     * @tparam T the class of clip.
     */

    /**
     * @class IClipSeries::ClipView
     * @brief The view of a generic audio clip.
     */

    /**
     * @fn IClipSeries::ClipView::ClipView()
     * Default constructor. Creates a null clip view.
     */

    /**
     * @fn bool IClipSeries::ClipView::isValid() const
     * Gets whether the clip view is valid. Null clip views and views of clip already deleted are invalid.
     *
     * Passing invalid clip view as an argument of IClipSeries member functions will cause undefined behaviors.
     */

    /**
     * @fn bool IClipSeries::ClipView::isNull() const
     * Gets whether the clip view is null.
     */

    /**
     * @fn T *IClipSeries::ClipView::content() const
     * Gets the content of the clip.
     */

    /**
     * @fn qint64 IClipSeries::ClipView::startPos() const
     * Gets the starting position of the clip content.
     */

    /**
     * @fn qint64 IClipSeries::ClipView::position() const
     * Gets the position of the clip in the clip series.
     */

    /**
     * @fn qint64 IClipSeries::ClipView::length() const
     * Gets the length of the clip.
     */

    /**
     * @fn bool IClipSeries::ClipView::operator==(const IClipSeries::ClipView &other) const
     * Equal-to operator overloading.
     */

    /**
     * @fn bool IClipSeries::ClipView::operator!=(const IClipSeries::ClipView &other) const
     * Not-equal-to operator overloading.
     */

    /**
     * @fn IClipSeries::ClipView IClipSeries::insertClip(T *content, qint64 position, qint64 startPos, qint64 length)
     * Inserts a clip to the clip series.
     *
     * If it fails (due to duplicated content, or other reasons), null clip view will be returned.
     */

    /**
     * @fn void IClipSeries::setClipStartPos(const IClipSeries::ClipView &clip, qint64 startPos)
     * Sets the starting position of a specified clip.
     */

    /**
     * @fn bool IClipSeries::setClipRange(const IClipSeries::ClipView &clip, qint64 position, qint64 length)
     * Sets the clip's position in the series and length.
     *
     * If it fails (due to some reasons), null clip view will be returned.
     */

    /**
     * @fn IClipSeries::ClipView IClipSeries::setClipContent(const IClipSeries::ClipView &clip, T *content)
     * Sets the content of a clip
     *
     * After setting, the old clip view becomes invalid.
     *
     * If it fails (due to duplicated content or other reasons), null clip view will be returned and the old clip view
     * remains valid.
     */

    /**
     * @fn IClipSeries::ClipView IClipSeries::findClip(T *content) const
     * Finds a clip by its content.
     *
     * If not found, a null clip view will be returned.
     */

    /**
     * @fn IClipSeries::ClipView IClipSeries::findClip(qint64 position) const
     * Finds a clip by the position in the series.
     *
     * If not found, a null clip view will be returned.
     */

    /**
     * @fn void IClipSeries::removeClip(const IClipSeries::ClipView &clip)
     * Removes a clip.
     */

    /**
     * @fn void IClipSeries::removeAllClips()
     * Removes all clips.
     */

    /**
     * @fn QList<ClipView> IClipSeries::clips() const
     * Gets all clips.
     */

    /**
     * @fn qint64 IClipSeries::effectiveLength() const
     * Gets the effective length (i.e. the ending position of the last clip).
     */

} // talcs