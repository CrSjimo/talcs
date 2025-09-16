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

    namespace ClipViewPrivate {
        ClipViewImpl::ClipViewImpl() : d(nullptr), k(0) {

        }

        bool ClipViewImpl::isValid() const {
            return k && d->clipContentDict.contains(k);
        }

        void *ClipViewImpl::content() const {
            Q_ASSERT(isValid());
            return d->clipContentDict.value(k);
        }

        qint64 ClipViewImpl::startPos() const {
            Q_ASSERT(isValid());
            return d->clipStartPosDict.value(k);
        }

        qint64 ClipViewImpl::position() const {
            Q_ASSERT(isValid());
            return d->clipPositionDict.value(k);
        }

        qint64 ClipViewImpl::length() const {
            Q_ASSERT(isValid());
            return d->intervalLookup(position(), content()).length();
        }

        ClipViewImpl::ClipViewImpl(const IClipSeriesPrivate *d, qint64 k) : d(d), k(k) {

        }
    }

    ClipViewPrivate::ClipViewImpl IClipSeriesPrivate::insertClip(void *content, qint64 position, qint64 startPos, qint64 length) {
        if (clipContentSet.contains(content))
            return {};
        auto interval = ClipInterval(content, position, length);
        clips.insert(interval);
        auto k = nextKey();
        clipPositionDict.insert(k, position);
        clipStartPosDict.insert(k, startPos);
        clipContentDict.insert(k, content);
        clipKeyDict.insert(content, k);
        clipContentSet.insert(content);
        endSet.insert(position + length);
        return ClipViewPrivate::ClipViewImpl(this, k);
    }

    void IClipSeriesPrivate::setClipStartPos(const ClipViewPrivate::ClipViewImpl &clipViewImpl, qint64 startPos) {
        Q_ASSERT(clipViewImpl.isValid());
        clipStartPosDict[clipViewImpl.k] = startPos;
    }

    bool IClipSeriesPrivate::setClipRange(const ClipViewPrivate::ClipViewImpl &clipViewImpl, qint64 position, qint64 length) {
        auto it = findClipIterator(clipPositionDict.value(clipViewImpl.k), clipViewImpl.content());
        auto oldPosition = it->interval().position();
        auto oldLength = it->interval().length();
        clips.erase(it);
        auto interval = IClipSeriesPrivate::ClipInterval(clipViewImpl.content(), position, length);
        clips.insert(interval);
        clipPositionDict[clipViewImpl.k] = position;
        endSet.erase(oldPosition + oldLength);
        endSet.insert(position + length);
        return true;
    }

    bool IClipSeriesPrivate::setClipContent(const ClipViewPrivate::ClipViewImpl &clipViewImpl, void *content) {
        if (content == clipViewImpl.content())
            return true;
        if (clipContentSet.contains(content))
            return {};
        auto it = findClipIterator(clipViewImpl.position(), clipViewImpl.content());
        auto newInterval = ClipInterval(content, clipViewImpl.position(), it.interval().length());
        clips.erase(it);
        clipContentSet.remove(clipViewImpl.content());
        clipKeyDict.remove(clipViewImpl.content());
        clipContentDict[clipViewImpl.k] = content;
        clipKeyDict[content] = clipViewImpl.k;
        clipContentSet.insert(content);
        clips.insert(newInterval);
        return true;
    }

    ClipViewPrivate::ClipViewImpl IClipSeriesPrivate::findClipByContent(void *content) const {
        return ClipViewPrivate::ClipViewImpl(this, clipKeyDict.value(content));
    }

    void IClipSeriesPrivate::findClipByPosition(qint64 position, const std::function<bool(const ClipViewPrivate::ClipViewImpl &)> &onFind) const {
        clips.overlap_find_all({nullptr, position, 1}, [&](const ClipIntervalTree::const_iterator &it) {
            return onFind(ClipViewPrivate::ClipViewImpl(this, clipKeyDict.value(it->interval().content())));
        });
    }

    void IClipSeriesPrivate::removeClip(const ClipViewPrivate::ClipViewImpl &clipViewImpl) {
        auto pos = clipViewImpl.position();
        auto it = findClipIterator(pos, clipViewImpl.content());
        auto endPos = pos + it->interval().length();
        clips.erase(it);
        clipPositionDict.remove(clipViewImpl.k);
        clipStartPosDict.remove(clipViewImpl.k);
        clipContentSet.remove(clipContentDict.take(clipViewImpl.k));
        endSet.erase(endPos);
    }

    void IClipSeriesPrivate::removeAllClips() {
        clips.clear();
        clipPositionDict.clear();
        clipStartPosDict.clear();
        clipContentDict.clear();
        clipContentSet.clear();
        endSet.clear();
    }

    QList<ClipViewPrivate::ClipViewImpl> IClipSeriesPrivate::clipViewImplList() const {
        QList<ClipViewPrivate::ClipViewImpl> list;
        for (auto p = clips.cbegin(); p != clips.cend(); p++) {
            list.append(ClipViewPrivate::ClipViewImpl(this, clipKeyDict.value(p->interval().content())));
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
     * If it fails (due to some reasons), false will be returned.
     */

    /**
     * @fn bool IClipSeries::setClipContent(const IClipSeries::ClipView &clip, T *content)
     * Sets the content of a clip
     *
     * If it fails (due to duplicated content or other reasons), false will be returned
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
