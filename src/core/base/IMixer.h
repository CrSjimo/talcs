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

#ifndef TALCS_IMIXER_H
#define TALCS_IMIXER_H

#include <list>

#include <QList>

namespace talcs {
    template <class T>
    struct IMixerPrivate;

    template <class T>
    struct IMixer {
        class SourceIterator {
        public:
            SourceIterator() = default;

            inline SourceIterator next() const {
                return SourceIterator(std::next(m_it), m_lis);
            }
            inline SourceIterator previous() const {
                return SourceIterator(std::prev(m_it), m_lis);
            }
            inline T *data() const {
                if (m_it == m_lis->cend())
                    return nullptr;
                return *m_it;
            }
            inline bool operator==(const SourceIterator &other) const {
                return this->data() == other.data();
            }
            inline bool operator!=(const SourceIterator &other) const {
                return this->data() != other.data();
            }
        private:
            friend class IMixer<T>;
            friend class IMixerPrivate<T>;

            using ListType = std::list<T *>;
            using IteratorType = typename ListType::const_iterator;

            inline SourceIterator(IteratorType it, const ListType *lis) : m_it(it), m_lis(lis) {
            }

            IteratorType m_it;
            const ListType *m_lis = nullptr;
        };

        virtual bool addSource(T *src, bool takeOwnership) = 0;
        virtual SourceIterator appendSource(T *src, bool takeOwnership) = 0;
        virtual SourceIterator prependSource(T *src, bool takeOwnership) = 0;
        virtual SourceIterator insertSource(const SourceIterator &pos, T *src, bool takeOwnership) = 0;

        virtual bool removeSource(T *src) = 0;
        virtual void eraseSource(const SourceIterator &srcIt) = 0;
        virtual void removeAllSources() = 0;

        virtual void moveSource(const SourceIterator &pos, const SourceIterator &target) = 0;
        virtual void swapSource(const SourceIterator &first, const SourceIterator &second) = 0;

        virtual QList<T *> sources() const = 0;
        virtual SourceIterator firstSource() const = 0;
        virtual SourceIterator lastSource() const = 0;
        virtual SourceIterator findSource(T *src) const = 0;

        virtual SourceIterator nullIterator() const = 0;

        virtual void setSourceSolo(T *src, bool isSolo) = 0;
        virtual bool isSourceSolo(T *src) const = 0;

        virtual void setGain(float gain) = 0;
        virtual float gain() const = 0;

        virtual void setPan(float pan) = 0;
        virtual float pan() const = 0;

        virtual void setRouteChannels(bool routeChannels) = 0;
        virtual bool routeChannels() const = 0;

        virtual void setSilentFlags(int silentFlags) = 0;
        virtual int silentFlags() const = 0;

        virtual void setLevelMeterChannelCount(int count) = 0;
        virtual int levelMeterChannelCount() = 0;
    };
    
}

#endif // TALCS_IMIXER_H
