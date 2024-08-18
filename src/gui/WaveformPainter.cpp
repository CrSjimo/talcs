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

#include <TalcsCore/PositionableAudioSource.h>

namespace talcs {
    WaveformPainter::WaveformPainter(QObject *parent) : QObject(parent), d_ptr(new WaveformPainterPrivate) {
        Q_D(WaveformPainter);
        d->q_ptr = this;
        d->worker.d = d;
        d->worker.moveToThread(&d->workerThread);
        d->workerThread.start();
    }

    WaveformPainter::~WaveformPainter() {
        Q_D(WaveformPainter);
        d->isInterrupted = true;
        d->workerThread.quit();
        d->workerThread.wait();
    }

    void WaveformPainter::setSource(PositionableAudioSource *src, int channelCount, qint64 length, bool mergeChannels) {
        Q_D(WaveformPainter);

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

    template <typename T, typename S>
    constexpr auto divideCeiling(T a, S b) {
        return a / b + (a % b != 0);
    }

    void mergeChannels(AudioBuffer &buf, qint64 sampleCount) {
        for (int ch = 0; ch < buf.channelCount(); ch++) {
            if (ch == 0)
                std::transform(buf.readPointerTo(0, 0), buf.readPointerTo(0, sampleCount), buf.writePointerTo(0, 0), [ch](float v) {
                    return static_cast<float>(v / static_cast<double>(ch));
                });
            else
                std::transform(buf.readPointerTo(ch, 0), buf.readPointerTo(ch, sampleCount), buf.readPointerTo(0, 0), buf.writePointerTo(0, 0), [ch](float v, float v0) {
                    return static_cast<float>(v / static_cast<double>(ch)) + v0;
                });
        }

    }

    void WaveformPainterWorker::work(qint64 startPos, qint64 length) {
        auto initialIndex16 = startPos / 16;
        length += startPos - initialIndex16 * 16;
        startPos = initialIndex16 * 16;
        d->src->setNextReadPosition(startPos);

        auto index16 = initialIndex16;
        for (; startPos < length; startPos += d->buf.sampleCount()) {
            auto actualReadLength = std::min(d->buf.sampleCount(), length - startPos);
            d->src->read({&d->buf, 0, actualReadLength});
            if (d->mergeChannels)
                mergeChannels(d->buf, actualReadLength);
            for (qint64 sampleIndex = 0; sampleIndex < actualReadLength; sampleIndex += 16, index16++) {
                for (int ch = 0; ch < (d->mergeChannels ? 1 : d->buf.channelCount()); ch++) {
                    auto bufMinMax = d->buf.findMinMax(ch, index16, std::min(actualReadLength - sampleIndex, qint64(16)));
                    d->mipmap16[ch][index16].first = static_cast<qint8>(qBound(-128, static_cast<int>(std::floor(bufMinMax.first * 127.0)), 127));
                    d->mipmap16[ch][index16].second = static_cast<qint8>(qBound(-128, static_cast<int>(std::ceil(bufMinMax.second * 127.0)), 127));
                }
            }
        }

        auto index256 = startPos / 256;
        auto length256 = divideCeiling(length, 256);
        for (qint64 i = 0; i < length256; i++) {
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
        for (qint64 i = 0; i < length4096; i++) {
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
        auto length = static_cast<qint64>(std::round(startPosSecond * d->src->sampleRate()));
        QPair<qint8, qint8> ret;
        const QVector<QPair<qint8, qint8>> *mipmap;
        int mipmapSize;
        if (length >= 4096) {
            mipmap = &d->mipmap4096[channel];
            mipmapSize = 4096;
        } else if (length >= 256) {
            mipmap = &d->mipmap256[channel];
            mipmapSize = 256;
        } else {
            mipmap = &d->mipmap16[channel];
            mipmapSize = 16;
        }
        auto initialIndex = startPos / mipmapSize;
        for (auto i = initialIndex; i < (startPos + length) / mipmapSize; i++) {
            ret.first = std::min(ret.first, (*mipmap)[i - initialIndex].first);
            ret.second = std::max(ret.second, (*mipmap)[i - initialIndex].second);
        }
        return ret;
    }

    void WaveformPainter::paint(QPainter *painter, const QRect &rect, double startPosSecond, double lengthSecond, int channel, double verticalScale) const {
        Q_D(const WaveformPainter);
        auto unitLengthSecond = lengthSecond / rect.width();
        auto unitLength = static_cast<qint64>(std::round(unitLengthSecond));
        const QVector<QPair<qint8, qint8>> *mipmap;
        int mipmapSize;
        if (unitLength >= 4096) {
            mipmap = &d->mipmap4096[channel];
            mipmapSize = 4096;
        } else if (unitLength >= 256) {
            mipmap = &d->mipmap256[channel];
            mipmapSize = 256;
        } else {
            mipmap = &d->mipmap16[channel];
            mipmapSize = 16;
        }
        for (int x = rect.left(); x <= rect.right(); x++) {
            QPair<qint8, qint8> ret;
            auto startPos = static_cast<qint64>(std::round(startPosSecond * d->src->sampleRate()));
            auto initialIndex = startPos / mipmapSize;
            for (auto i = initialIndex; i < (startPos + unitLength) / mipmapSize; i++) {
                ret.first = std::min(ret.first, (*mipmap)[i - initialIndex].first);
                ret.second = std::max(ret.second, (*mipmap)[i - initialIndex].second);
            }
            startPosSecond += unitLengthSecond;
            painter->drawLine(QLineF(x, rect.top() + 1.0 / 255.0 * (255 - ret.first), x, rect.top() + 1.0 / 255.0 * (255 - ret.second)));
        }
    }

}