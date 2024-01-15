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
    ClipViewImpl::ClipViewImpl() : d(nullptr), m_content(0) {

    }

    bool ClipViewImpl::isValid() const {
        return m_content && d->clipPositionDict.contains(m_content);
    }

    qintptr ClipViewImpl::content() const {
        return m_content;
    }

    qint64 ClipViewImpl::startPos() const {
        Q_ASSERT(isValid());
        return d->clipStartPosDict.value(m_content);
    }

    void ClipViewImpl::setStartPos(qint64 startPos) {
        Q_ASSERT(isValid());
        d->clipStartPosDict[m_content] = startPos;
    }

    qint64 ClipViewImpl::position() const {
        Q_ASSERT(isValid());
        return d->clipPositionDict.value(m_content);
    }

    qint64 ClipViewImpl::length() const {
        Q_ASSERT(isValid());
        return d->intervalLookup(position()).length();
    }

    bool ClipViewImpl::resetRange(qint64 position, qint64 length) {
        Q_ASSERT(isValid());
        return d->resetClipRange(m_content, position, length);
    }

    ClipViewImpl::ClipViewImpl(IClipSeriesPrivate *d, qintptr content) : d(d), m_content(content) {

    }

    ClipViewImpl IClipSeriesPrivate::insertClip(qintptr content, qint64 position, qint64 startPos, qint64 length) {
        auto ival = ClipInterval(content, position, length);
        if (qAsConst(clips).overlap_find(ival) != clips.cend())
            return nullClipViewImpl();
        clips.insert(ival);
        clipPositionDict.insert(content, position);
        clipStartPosDict.insert(content, startPos);
        endSet.insert(position + length);
        return ClipViewImpl(this, content);
    }

    ClipViewImpl IClipSeriesPrivate::findClipByContent(qintptr content) const {
        return ClipViewImpl(const_cast<IClipSeriesPrivate *>(this), content);
    }

    ClipViewImpl IClipSeriesPrivate::findClipByPosition(qint64 position) const {
        auto it = findClipIterator(position);
        if (it == clips.cend())
            return nullClipViewImpl();
        return ClipViewImpl(const_cast<IClipSeriesPrivate *>(this), it->interval().content());
    }

    ClipViewImpl IClipSeriesPrivate::nullClipViewImpl() const {
        return ClipViewImpl(const_cast<IClipSeriesPrivate *>(this), 0);
    }

    void IClipSeriesPrivate::removeClip(const ClipViewImpl &clipViewImpl) {
        auto pos = clipViewImpl.position();
        auto it = findClipIterator(pos);
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
            list.append(ClipViewImpl(const_cast<IClipSeriesPrivate *>(this), p->interval().content()));
        }
        return list;
    }

    qint64 IClipSeriesPrivate::effectiveLength() const {
        if (endSet.empty())
            return 0;
        return *endSet.rbegin();
    }

    bool IClipSeriesPrivate::resetClipRange(qintptr content, qint64 newPosition, qint64 newLength) {
        auto it = findClipIterator(clipPositionDict.value(content));
        auto oldPosition = it->interval().position();
        auto oldLength = it->interval().length();
        clips.erase(it);
        auto ival = ClipInterval(content, newPosition, newLength);
        if (qAsConst(clips).overlap_find(ival) != clips.cend()) {
            clips.insert({content, oldPosition, oldLength});
            return false;
        }
        clips.insert(ival);
        return true;
    }

    IClipSeriesPrivate::ClipInterval IClipSeriesPrivate::intervalLookup(qint64 pos) const {
        return findClipIterator(pos)->interval();
    }

    IClipSeriesPrivate::ClipIntervalTree::iterator IClipSeriesPrivate::findClipIterator(qint64 pos) {
        return clips.overlap_find({0, pos, 1});
    }

    IClipSeriesPrivate::ClipIntervalTree::const_iterator IClipSeriesPrivate::findClipIterator(qint64 pos) const {
        return clips.overlap_find({0, pos, 1});
    }

} // talcs