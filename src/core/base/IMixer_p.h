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

#ifndef TALCS_IMIXER_P_H
#define TALCS_IMIXER_P_H

#include <QHash>
#include <QList>
#include <QMutex>

#include <TalcsCore/AudioBuffer.h>
#include <TalcsCore/IMixer.h>

namespace talcs {

    static inline QPair<float, float> applyGainAndPan(float gain, float pan) {
        return {gain * qMin(1.0f, 1.0f - pan), gain * qMin(1.0f, 1.0f + pan)};
    }

    template <class T>
    struct SourceInfo {
        T *src;
        bool takeOwnership = false;
        bool isSolo = false;

        inline bool operator==(const SourceInfo<T> &other) const {
            return src == other.src;
        }
    };

    template <class T>
    struct IMixerPrivate {

        using SrcIt = typename IMixer<T>::SourceIterator;

        QHash<T *, SourceInfo<T>> sourceDict;
        std::list<T *> sourceList;

        QMutex mutex;

        float gain = 1;
        float pan = 0;
        int silentFlags = 0;

        AudioBuffer tmpBuf;

        QVector<float> currentMagnitudes;

        bool routeChannels = false;

        void deleteOwnedSources() const {
            for (auto [src, takeOwnership, _] : sourceDict) {
                if (takeOwnership) {
                    delete src;
                }
            }
        }

        SrcIt insertSource(const SrcIt &pos, T *src, bool takeOwnership, bool isOpen, qint64 bufferSize, double sampleRate) {
            if (sourceDict.contains(src))
                return SrcIt(sourceList.end(), &sourceList);
            if (isOpen && !src->open(bufferSize, sampleRate))
                return SrcIt(sourceList.end(), &sourceList);
            sourceDict.insert(src, {src, takeOwnership});
            return SrcIt(sourceList.insert(pos.m_it, src), &sourceList);
        }

        SrcIt sourceIteratorEnd() const {
            return SrcIt(sourceList.end(), &sourceList);
        }

        void eraseSource(const SrcIt &pos) {
            setSourceSolo(pos.data(), false);
            sourceDict.remove(pos.data());
            sourceList.erase(pos.m_it);
        }

        SrcIt findSource(T *src) const {
            return SrcIt(std::find(sourceList.cbegin(), sourceList.cend(), src), &sourceList);
        }

        void moveSource(const SrcIt &pos, const SrcIt &target) {
            sourceList.splice(pos.m_it, sourceList, target.m_it);
        }

        void swapSource(const SrcIt &first, const SrcIt &second) {
            auto pos2 = second.m_it;
            pos2++;
            sourceList.splice(first.m_it, sourceList, second.m_it);
            sourceList.splice(pos2, sourceList, first.m_it);
        }

        bool addSource(T *src, bool takeOwnership, bool isOpen, qint64 bufferSize, double sampleRate) {
            return insertSource(sourceIteratorEnd(), src, takeOwnership, isOpen, bufferSize, sampleRate).m_it != sourceList.end();
        }

        bool removeSource(T *src) {
            auto srcIt = findSource(src);
            if (!srcIt.data())
                return false;
            eraseSource(srcIt);
            return true;
        }

        void removeAllSources() {
            sourceList.clear();
            sourceDict.clear();
            soloCounter = 0;
        }

        QList<T *> sources() const {
            return QList<T *>(sourceList.cbegin(), sourceList.cend());
        }

        SrcIt firstSource() const {
            return SrcIt(sourceList.cbegin(), &sourceList);
        }

        SrcIt lastSource() const {
            if (sourceList.empty())
                return sourceIteratorEnd();
            return sourceIteratorEnd().previous();
        }

        int soloCounter = 0;

        void setSourceSolo(T *src, bool isSolo) {
            auto it = sourceDict.find(src);
            if (it == sourceDict.end())
                return;
            if (it->isSolo == isSolo)
                return;
            it->isSolo = isSolo;
            soloCounter += (isSolo ? 1 : -1);
        }

        bool isSourceSolo(T *src) const {
            auto it = sourceDict.find(src);
            if (it == sourceDict.end())
                return false;
            return it->isSolo;
        }

        bool isMutedBySoloSetting(T *src) const {
            if (soloCounter == 0)
                return false;
            auto it = sourceDict.find(src);
            if (it == sourceDict.end())
                return false;
            return !it->isSolo;
        }

        bool start(qint64 bufferSize, double sampleRate) {
            if (std::all_of(sourceList.cbegin(), sourceList.cend(),
                            [=](T *src) { return src->open(bufferSize, sampleRate); })) {
                tmpBuf.resize(2, bufferSize);
                return true;
            } else {
                return false;
            }
        }

        void stop() {
            std::for_each(sourceList.cbegin(), sourceList.cend(), [=](T *src) { src->close(); });
            tmpBuf.resize(0, 0);
        }

        qint64 mix(const AudioSourceReadData &readData, qint64 readLength) {
            auto channelCount = readData.buffer->channelCount();
            auto gainLeftRight = applyGainAndPan(gain, pan);
            int routeCnt = 0;
            qint64 actualReadLength = 0;
            for (auto src: sourceList) {
                auto srcInfo = sourceDict.value(src);
                bool isMutedBySoloSetting = (soloCounter && !srcInfo.isSolo);
                tmpBuf.clear();
                actualReadLength = qMax(
                        src->read(AudioSourceReadData(&tmpBuf, 0, readLength, isMutedBySoloSetting ? -1 : silentFlags)),
                        actualReadLength);
                if (isMutedBySoloSetting)
                    tmpBuf.clear();

                if (routeChannels) {
                    if (routeCnt >= channelCount / 2)
                        break;
                    if ((1 & silentFlags) == 0)
                        readData.buffer->addSampleRange(routeCnt * 2, readData.startPos, readLength, tmpBuf, 0, 0,
                                                        gainLeftRight.first);
                    if ((2 & silentFlags) == 0)
                        readData.buffer->addSampleRange(routeCnt * 2 + 1, readData.startPos, readLength, tmpBuf, 1, 0,
                                                        gainLeftRight.second);
                    routeCnt++;
                } else {
                    for (int i = 0; i < channelCount; i++) {
                        if (((1 << i) & silentFlags) != 0)
                            continue;
                        auto gainCh = i == 0 ? gainLeftRight.first : i == 1 ? gainLeftRight.second : gain;
                        readData.buffer->addSampleRange(i, readData.startPos, readLength, tmpBuf, i, 0, gainCh);
                    }
                }
            }
            if (routeChannels) {
                for (int ch = routeCnt * 2; ch < readData.buffer->channelCount(); ch++) {
                    readData.buffer->clear(ch, readData.startPos, readData.length);
                }
            }

            // update the level meter
            for (int i = 0; i < currentMagnitudes.size(); i++) {
                if (i < readData.buffer->channelCount())
                    currentMagnitudes[i] = readData.buffer->magnitude(i, readData.startPos, readLength);
                else
                    currentMagnitudes[i] = 0.0f;
            }

            return actualReadLength;
        }
    };
    
}

#endif // TALCS_IMIXER_P_H
