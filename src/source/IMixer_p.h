#ifndef TALCS_IMIXER_P_H
#define TALCS_IMIXER_P_H

#include <QMutex>

#include <QMChronMap.h>

#include "buffer/AudioBuffer.h"

namespace talcs {
    static inline QPair<float, float> applyGainAndPan(float gain, float pan) {
        return {gain * std::max(1.0f, 1.0f - pan), gain * std::max(1.0f, 1.0f + pan)};
    }

    struct SourceInfo {
        bool takeOwnership = false;
        bool isSolo = false;
    };

    template <class T>
    struct IMixerPrivate {
        QMChronMap<T *, SourceInfo> sourceDict;
        QMutex mutex;

        float gain = 1;
        float pan = 0;
        int silentFlags = 0;

        AudioBuffer tmpBuf;

        bool routeChannels = false;

        void deleteOwnedSources() const {
            for (auto src : sourceDict.keys()) {
                if (sourceDict.value(src).takeOwnership) {
                    delete src;
                }
            }
        }

        bool addSource(T *src, bool takeOwnership, bool isOpen, qint64 bufferSize, double sampleRate) {
            if (sourceDict.contains(src))
                return false;
            if (isOpen && !src->open(bufferSize, sampleRate))
                return false;
            sourceDict.append(src, {takeOwnership});
            return true;
        }

        bool removeSource(T *src) {
            if (sourceDict.remove(src)) {
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
            return sourceDict.keys();
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
            return sourceDict.value(src).isSolo;
        }

        bool start(qint64 bufferSize, double sampleRate) {
            auto sourceList = sourceDict.keys();
            if (std::all_of(sourceList.constBegin(), sourceList.constEnd(),
                            [=](AudioSource *src) { return src->open(bufferSize, sampleRate); })) {
                tmpBuf.resize(2, bufferSize);
                return true;
            } else {
                return false;
            }
        }

        void stop() {
            auto sourceList = sourceDict.keys();
            std::for_each(sourceList.constBegin(), sourceList.constEnd(), [=](AudioSource *src) { src->close(); });
            tmpBuf.resize(0, 0);
        }

        qint64 mix(const AudioSourceReadData &readData, qint64 readLength) {
            auto channelCount = readData.buffer->channelCount();
            auto gainLeftRight = applyGainAndPan(gain, pan);
            int routeCnt = 0;
            // TODO tell QtMediate to use key value iterator
            for (auto src : sourceDict.keys()) {
                auto srcInfo = sourceDict.value(src);
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
