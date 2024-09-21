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

#ifndef TALCS_ICLIPSERIES_H
#define TALCS_ICLIPSERIES_H

#include <TalcsCore/TalcsCoreGlobal.h>

namespace talcs {

    class IClipSeriesPrivate;

    class TALCSCORE_EXPORT ClipViewImpl {
    public:
        ClipViewImpl();

        bool isValid() const;

        bool isNull() const {
            return !bool(m_content);
        }

        qintptr content() const;

        qint64 startPos() const;

        qint64 position() const;

        qint64 length() const;

        template<class ClipView>
        ClipViewImpl(const ClipView &clipView) : ClipViewImpl(clipView.m_impl) {
        }

    private:
        friend class IClipSeriesPrivate;
        explicit ClipViewImpl(const IClipSeriesPrivate *d, qintptr content);
        const IClipSeriesPrivate *d;
        qintptr m_content;
    };

    template <class T>
    class IClipSeries {
    public:
        class ClipView {
        public:
            ClipView() = default;

            ClipView(const ClipViewImpl &impl) : m_impl(impl) {
            }

            bool isValid() const {
                return m_impl.isValid();
            }

            bool isNull() const {
                return m_impl.isNull();
            }

            T *content() const {
                return reinterpret_cast<T *>(m_impl.content());
            }

            qint64 startPos() const {
                return m_impl.startPos();
            }

            qint64 position() const {
                return m_impl.position();
            }

            qint64 length() const {
                return m_impl.length();
            }

            bool operator==(const ClipView &other) const {
                return content() == other.content();
            }

            bool operator!=(const ClipView &other) const {
                return content() != other.content();
            }
        private:
            friend class ClipViewImpl;
            ClipViewImpl m_impl;
        };

        virtual ClipView insertClip(T *content, qint64 position, qint64 startPos, qint64 length) = 0;
        virtual void setClipStartPos(const ClipView &clip, qint64 startPos) = 0;
        virtual bool setClipRange(const ClipView &clip, qint64 position, qint64 length) = 0;
        virtual ClipView setClipContent(const ClipView &clip, T *content) = 0;

        virtual ClipView findClip(T *content) const = 0;
        virtual QList<ClipView> findClip(qint64 position) const = 0;

        virtual void removeClip(const ClipView &clip) = 0;
        virtual void removeAllClips() = 0;

        virtual QList<ClipView> clips() const = 0;

        virtual qint64 effectiveLength() const = 0;

    protected:
        ~IClipSeries() = default;

    };

} // talcs

#endif //TALCS_ICLIPSERIES_H
