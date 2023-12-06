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

#include <QMutex>

#include <QList>

#include <TalcsCore/AudioBuffer.h>

namespace talcs {

    static inline QPair<float, float> applyGainAndPan(float gain, float pan) {
        return {gain * std::max(1.0f, 1.0f - pan), gain * std::max(1.0f, 1.0f + pan)};
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
        QList<SourceInfo<T>> sourceDict;
        QMutex mutex;

        float gain = 1;
        float pan = 0;
        int silentFlags = 0;
        bool isMeterEnabled = false;

        AudioBuffer tmpBuf;

        bool routeChannels = false;

        void deleteOwnedSources() const {
            for (auto [src, takeOwnership, _] : sourceDict) {
                if (takeOwnership) {
                    delete src;
                }
            }
        }

        bool addSource(T *src, bool takeOwnership, bool isOpen, qint64 bufferSize, double sampleRate) {
            if (isOpen && !src->open(bufferSize, sampleRate))
                return false;
            sourceDict.append({src, takeOwnership});
            return true;
        }

        bool removeSource(T *src) {
            if (sourceDict.removeOne({src})) {
                src->close();
                return true;
            }
            return false;
        }

        void removeAllSources() {
            stop();
            sourceDict.clear();
        }

        QList<T *> sources() const {
            QList<T *> list;
            for (auto [src, _0, _1]: sourceDict) {
                list.append(src);
            }
            return list;
        }

        int soloCounter = 0;

        void setSourceSolo(T *src, bool isSolo) {
            auto i = sourceDict.indexOf({src});
            if (i == -1)
                return;
            if (sourceDict[i].isSolo == isSolo)
                return;
            sourceDict[i].isSolo = isSolo;
            soloCounter += (isSolo ? 1 : -1);
        }

        bool isSourceSolo(T *src) const {
            auto i = sourceDict.indexOf({src});
            if (i == -1)
                return false;
            return sourceDict[i].isSolo;
        }

        bool start(qint64 bufferSize, double sampleRate) {
            if (std::all_of(sourceDict.constBegin(), sourceDict.constEnd(),
                            [=](const SourceInfo<T> &srcInfo) { return srcInfo.src->open(bufferSize, sampleRate); })) {
                tmpBuf.resize(2, bufferSize);
                return true;
            } else {
                return false;
            }
        }

        void stop() {
            std::for_each(sourceDict.constBegin(), sourceDict.constEnd(), [=](const SourceInfo<T> &srcInfo) { srcInfo.src->close(); });
            tmpBuf.resize(0, 0);
        }

        qint64 mix(const AudioSourceReadData &readData, qint64 readLength) {
            auto channelCount = readData.buffer->channelCount();
            auto gainLeftRight = applyGainAndPan(gain, pan);
            int routeCnt = 0;
            for (auto &srcInfo : sourceDict) {
                auto src = srcInfo.src;
                bool isMutedBySoloSetting = (soloCounter && !srcInfo.isSolo);
                readLength = std::min(
                    readLength,
                    src->read(AudioSourceReadData(&tmpBuf, 0, readLength, isMutedBySoloSetting ? -1 : silentFlags)));
                if (isMutedBySoloSetting)
                    tmpBuf.clear();

                if (routeChannels) {
                    if (routeCnt >= channelCount / 2)
                        break;
                    readData.buffer->addSampleRange(routeCnt * 2, readData.startPos, readLength, tmpBuf, 0, 0,
                                                    gainLeftRight.first);
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
            return readLength;
        }
    };
    
}

#endif // TALCS_IMIXER_P_H
