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

#ifndef TALCS_ICLIPSERIES_P_H
#define TALCS_ICLIPSERIES_P_H

#include <set>

#include <QHash>
#include <QScopedPointer>

#include <TalcsCore/IClipSeries.h>
#include <interval-tree/interval_tree.hpp>

namespace talcs {
    class TALCSCORE_EXPORT IClipSeriesPrivate {
    public:

        struct ClipInterval : public lib_interval_tree::interval<qint64> {
            inline ClipInterval(void * content, qint64 position, qint64 length) : lib_interval_tree::interval<qint64>(position, position + length - 1), m_content(content) {
            }

            inline void *content() const {
                return m_content;
            }

            inline qint64 position() const {
                return low();
            }

            inline qint64 length() const {
                return high() - low() + 1;
            }

            void *m_content;
        };

        using ClipIntervalTree = lib_interval_tree::interval_tree<ClipInterval>;
        ClipIntervalTree clips;
        QHash<void *, qint64> clipPositionDict;
        QHash<void *, qint64> clipStartPosDict;
        std::set<qint64> endSet;

        ClipViewImpl insertClip(void *content, qint64 position, qint64 startPos, qint64 length);
        void setClipStartPos(const ClipViewImpl &clipViewImpl, qint64 startPos);
        bool setClipRange(const ClipViewImpl &clipViewImpl, qint64 position, qint64 length);
        ClipViewImpl setClipContent(const ClipViewImpl &clipViewImpl, void *content);

        ClipViewImpl findClipByContent(void *content) const;
        void findClipByPosition(qint64 position, const std::function<bool(const ClipViewImpl &)> &onFind) const;

        void removeClip(const ClipViewImpl &clipViewImpl);
        void removeAllClips();

        QList<ClipViewImpl> clipViewImplList() const;

        qint64 effectiveLength() const;

        IClipSeriesPrivate::ClipInterval intervalLookup(qint64 pos, void *content) const;
        IClipSeriesPrivate::ClipIntervalTree::iterator findClipIterator(qint64 pos, void * content);
        IClipSeriesPrivate::ClipIntervalTree::const_iterator findClipIterator(qint64 pos, void *content) const;

    };

}

#endif //TALCS_ICLIPSERIES_P_H
