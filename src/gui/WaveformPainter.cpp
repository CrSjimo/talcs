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

#include "WaveformPainter.h"
#include "WaveformPainter_p.h"

#include <algorithm>

#include <QRect>
#include <QPainter>
#include <QDebug>

#include <TalcsCore/PositionableAudioSource.h>

namespace talcs {
    WaveformPainter::WaveformPainter(QObject *parent) : QObject(parent), d_ptr(new WaveformPainterPrivate) {
        Q_D(WaveformPainter);
        d->q_ptr = this;
        d->worker.d = d;
        d->worker.moveToThread(&d->workerThread);

        connect(&d->worker, &WaveformPainterWorker::workRequired, &d->worker, &WaveformPainterWorker::work);
    }

    WaveformPainter::~WaveformPainter() {
        Q_D(WaveformPainter);
        d->isInterrupted = true;
        d->workerThread.quit();
        d->workerThread.wait();
    }

    template <typename T, typename S>
    constexpr auto divideCeiling(T a, S b) {
        return a / b + (a % b != 0);
    }

    void WaveformPainter::setSource(PositionableAudioSource *src, int channelCount, qint64 length, bool mergeChannels) {
        Q_D(WaveformPainter);
        d->isInterrupted = true;
        d->workerThread.quit();
        d->workerThread.wait();
        d->src = src;
        d->channelCount = channelCount;
        d->length = length;
        d->mergeChannels = mergeChannels;

        d->mipmap16.resize(mergeChannels ? 1 : channelCount);
        for (auto &mipmap : d->mipmap16) {
            mipmap.clear();
            mipmap.resize(divideCeiling(length, 16));
        }
        d->mipmap256.resize(mergeChannels ? 1 : channelCount);
        for (auto &mipmap : d->mipmap256) {
            mipmap.clear();
            mipmap.resize(divideCeiling(length, 256));
        }
        d->mipmap4096.resize(mergeChannels ? 1 : channelCount);
        for (auto &mipmap : d->mipmap4096) {
            mipmap.clear();
            mipmap.resize(divideCeiling(length, 4096));
        }

        d->buf.resize(channelCount, src->bufferSize() / 16 * 16);

        d->workerThread.start();
    }

    void WaveformPainter::startLoad(qint64 startPosHint, qint64 length) {
        Q_D(WaveformPainter);
        d->isInterrupted = false;
        emit d->worker.workRequired(startPosHint, length);
    }

    void WaveformPainter::interruptLoad() {
        Q_D(WaveformPainter);
        d->isInterrupted = true;
    }

    void mergeChannels(AudioBuffer &buf, qint64 sampleCount) {
        int chCnt = buf.channelCount();
        for (int ch = 0; ch < chCnt; ch++) {
            if (ch == 0)
                std::transform(buf.readPointerTo(0, 0), buf.readPointerTo(0, sampleCount), buf.writePointerTo(0, 0), [chCnt](float v) {
                    return static_cast<float>(v / static_cast<double>(chCnt));
                });
            else
                std::transform(buf.readPointerTo(ch, 0), buf.readPointerTo(ch, sampleCount), buf.readPointerTo(0, 0), buf.writePointerTo(0, 0), [chCnt](float v, float v0) {
                    return static_cast<float>(v / static_cast<double>(chCnt)) + v0;
                });
        }

    }

    void WaveformPainterWorker::work(qint64 startPos, qint64 length) {
        auto initialIndex16 = startPos / 16;
        length += startPos - initialIndex16 * 16;
        startPos = initialIndex16 * 16;
        d->src->setNextReadPosition(startPos);

        auto index16 = initialIndex16;
        for (auto curPos = startPos; curPos < startPos + length; curPos += d->buf.sampleCount()) {
            if (d->isInterrupted)
                return;
            auto actualReadLength = std::min(d->buf.sampleCount(), startPos + length - curPos);
            d->src->read({&d->buf, 0, actualReadLength});
            if (d->mergeChannels)
                mergeChannels(d->buf, actualReadLength);
            for (qint64 sampleIndex = 0; sampleIndex < actualReadLength; sampleIndex += 16, index16++) {
                for (int ch = 0; ch < (d->mergeChannels ? 1 : d->buf.channelCount()); ch++) {
                    auto bufMinMax = d->buf.findMinMax(ch, sampleIndex, std::min(actualReadLength - sampleIndex, qint64(16)));
                    d->mipmap16[ch][index16].first = static_cast<qint8>(qBound(-128, static_cast<int>(std::floor((bufMinMax.first + 1.0f) * 127.5)) - 128, 127));
                    d->mipmap16[ch][index16].second = static_cast<qint8>(qBound(-128, static_cast<int>(std::ceil((bufMinMax.second + 1.0f) * 127.5)) - 128, 127));
                }
            }
        }

        auto index256 = startPos / 256;
        auto length256 = divideCeiling(length, 256);
        for (qint64 i = 0; i < length256; i++, index256++) {
            if (d->isInterrupted)
                return;
            for (int ch = 0; ch < (d->mergeChannels ? 1 : d->buf.channelCount()); ch++) {
                auto firstIt = d->mipmap16[ch].begin() + index256 * 16;
                d->mipmap256[ch][index256].first = std::min_element(firstIt, firstIt + std::min<qint64>(d->mipmap16[ch].end() - firstIt, 16), [](auto a, auto b) {
                    return a.first < b.first;
                })->first;
                d->mipmap256[ch][index256].second = std::max_element(firstIt, firstIt + std::min<qint64>(d->mipmap16[ch].end() - firstIt, 16), [](auto a, auto b) {
                    return a.first < b.first;
                })->second;
            }
        }

        auto index4096 = startPos / 4096;
        auto length4096 = divideCeiling(length, 4096);
        for (qint64 i = 0; i < length4096; i++, index4096++) {
            if (d->isInterrupted)
                return;
            for (int ch = 0; ch < (d->mergeChannels ? 1 : d->buf.channelCount()); ch++) {
                auto firstIt = d->mipmap256[ch].begin() + index4096 * 16;
                d->mipmap4096[ch][index4096].first = std::min_element(firstIt, firstIt + std::min<qint64>(d->mipmap256[ch].end() - firstIt, 16), [](auto a, auto b) {
                    return a.first < b.first;
                })->first;
                d->mipmap4096[ch][index4096].second = std::max_element(firstIt, firstIt + std::min<qint64>(d->mipmap256[ch].end() - firstIt, 16), [](auto a, auto b) {
                    return a.first < b.first;
                })->second;
            }
        }

        emit d->q_func()->loadFinished(startPos, length);

    }

    QPair<qint8, qint8> WaveformPainter::getMinMaxFromMipmap(double startPosSecond, double lengthSecond, int channel) const {
        Q_D(const WaveformPainter);
        auto startPos = static_cast<qint64>(std::round(startPosSecond * d->src->sampleRate()));
        auto length = static_cast<qint64>(std::round(lengthSecond * d->src->sampleRate()));
        if (startPos >= d->length)
            return {0, 0};
        if (startPos + length > d->length)
            length = d->length - startPos;
        QPair<qint8, qint8> ret(127, -128);
        auto initialIndex = startPos / 16;
        auto endIndex = (startPos + length) / 16;
        if (endIndex == initialIndex)
            endIndex++;
        for (auto i = initialIndex; i < endIndex; i++) {
            if (i % 256 && endIndex - i > 256) {
                ret.first = std::min(ret.first, d->mipmap4096[channel][i / 256].first);
                ret.second = std::max(ret.second, d->mipmap4096[channel][i / 256].second);
                i += 256;
            } else if (i % 16 && endIndex - i > 16) {
                ret.first = std::min(ret.first, d->mipmap256[channel][i / 16].first);
                ret.second = std::max(ret.second, d->mipmap256[channel][i / 16].second);
                i += 16;
            } else {
                ret.first = std::min(ret.first, d->mipmap16[channel][i].first);
                ret.second = std::max(ret.second, d->mipmap16[channel][i].second);
                i += 1;
            }
        }
        return ret;
    }

    void WaveformPainter::paint(QPainter *painter, const QRect &rect, double startPosSecond, double lengthSecond, int channel, double verticalScale) const {
        Q_D(const WaveformPainter);
        auto unitLengthSecond = lengthSecond / rect.width();
        for (int x = rect.left(); x <= rect.right(); x++) {
            auto ret = getMinMaxFromMipmap(startPosSecond, unitLengthSecond, channel);
            startPosSecond += unitLengthSecond;
            painter->drawRect(QRectF(x, rect.top() + 1.0 / 255.0 * (128 - ret.second) * rect.height(), 1, rect.top() + 1.0 / 255.0 * (ret.second - ret.first) * rect.height()));
        }
    }

}