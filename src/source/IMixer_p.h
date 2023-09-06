#ifndef TALCS_IMIXER_P_H
#define TALCS_IMIXER_P_H

#include <QMutex>

#include <QMChronMap.h>

#include "buffer/AudioBuffer.h"

namespace talcs {
    static inline QPair<float, float> applyGainAndPan(float gain, float pan) {
        return {gain * std::max(1.0f, 1.0f - pan), gain * std::max(1.0f, 1.0f + pan)};
    }

    template <class T>
    struct IMixerPrivate {
        QMChronMap<T *, bool> sourceDict;
        QMutex mutex;

        float gain = 1;
        float pan = 0;

        AudioBuffer tmpBuf;

        bool routeChannels = false;

        void deleteOwnedSources() const {
            for (auto src : sourceDict.keys()) {
                if (sourceDict.value(src)) {
                    delete src;
                }
            }
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
            std::for_each(sourceList.constBegin(), sourceList.constEnd(),
                          [=](AudioSource *src) { src->close(); });
        }

        qint64 mix(const AudioSourceReadData &readData, qint64 readLength) {
            auto channelCount = readData.buffer->channelCount();
            auto gainLeftRight = applyGainAndPan(gain, pan);
            int routeCnt = 0;
            for (auto src : sourceDict.keys()) {
                readLength = std::min(readLength, src->read(AudioSourceReadData(&tmpBuf, 0, readLength)));
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
