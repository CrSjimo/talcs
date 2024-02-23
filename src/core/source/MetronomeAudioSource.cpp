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

#include "MetronomeAudioSource.h"
#include "MetronomeAudioSource_p.h"

#include <cmath>

#include <TalcsCore/AudioBuffer.h>
#include <TalcsCore/MemoryAudioSource.h>

namespace talcs {

    MetronomeAudioSource::MetronomeAudioSource() : MetronomeAudioSource(*new MetronomeAudioSourcePrivate) {

    }

    MetronomeAudioSource::~MetronomeAudioSource() {
        Q_D(MetronomeAudioSource);
        if (d->takeOwnershipOfMajor)
            delete d->majorBeatSource;
        if (d->takeOwnershipOfMinor)
            delete d->minorBeatSource;
    }

    MetronomeAudioSource::MetronomeAudioSource(MetronomeAudioSourcePrivate &d) : PositionableAudioSource(d) {
        MetronomeAudioSource::close();
    }

    bool MetronomeAudioSource::open(qint64 bufferSize, double sampleRate) {
        Q_D(MetronomeAudioSource);
        QMutexLocker locker(&d->mutex);
        if (d->majorBeatSource)
            if (!d->majorBeatSource->open(bufferSize, sampleRate))
                return false;
        if (d->minorBeatSource)
            if (!d->minorBeatSource->open(bufferSize, sampleRate))
                return false;
        return AudioStreamBase::open(bufferSize, sampleRate);
    }

    void MetronomeAudioSource::close() {
        Q_D(MetronomeAudioSource);
        QMutexLocker locker(&d->mutex);
        if (d->majorBeatSource)
            d->majorBeatSource->close();
        if (d->minorBeatSource)
            d->minorBeatSource->close();
        AudioStreamBase::close();
    }

    qint64 MetronomeAudioSource::read(const AudioSourceReadData &readData) {
        Q_D(MetronomeAudioSource);
        QMutexLocker locker(&d->mutex);
        if (!d->detector) {
            for (int ch = 0; ch < readData.buffer->channelCount(); ch++) {
                readData.buffer->clear(ch, readData.startPos, readData.length);
            }
            d->position += readData.length;
            return readData.length;
        }
        d->detector->detectInterval(d->position, readData.length);
        QPair<qint64, bool> beat = d->detector->nextBeat();
        qint64 tailLength = beat.first != -1 ? beat.first - d->position : readData.length;
        auto tailSrc = d->tailIsMajor ? d->majorBeatSource : d->tailIsMinor ? d->minorBeatSource : nullptr;
        if (tailSrc) {
            qint64 tailReadLength = tailSrc->read({readData.buffer, readData.startPos, tailLength, readData.silentFlags});
            if (tailReadLength < tailLength) {
                for (int ch = 0; ch < readData.buffer->channelCount(); ch++) {
                    readData.buffer->clear(ch, readData.startPos + tailReadLength, tailLength - tailReadLength);
                }
                d->tailIsMajor = d->tailIsMinor = false;
            }
        } else {
            for (int ch = 0; ch < readData.buffer->channelCount(); ch++) {
                readData.buffer->clear(ch, readData.startPos, tailLength);
            }
            d->tailIsMajor = d->tailIsMinor = false;
        }
        while (beat.first != -1) {
            qint64 beatStart = beat.first;
            bool isMajor = beat.second;
            beat = d->detector->nextBeat();
            qint64 beatLength = beat.first != -1 ? beat.first - beatStart : d->position + readData.length - beatStart;
            auto beatSource = isMajor ? d->majorBeatSource : d->minorBeatSource;
            d->tailIsMajor = isMajor;
            d->tailIsMinor = !isMajor;
            if (beatSource) {
                beatSource->setNextReadPosition(0);
                qint64 beatReadLength = beatSource->read({readData.buffer, readData.startPos + beatStart - d->position, beatLength, readData.silentFlags});
                if (beatReadLength < beatLength) {
                    for (int ch = 0; ch < readData.buffer->channelCount(); ch++) {
                        readData.buffer->clear(ch, readData.startPos + beatStart - d->position, beatLength - beatReadLength);
                    }
                    d->tailIsMajor = d->tailIsMinor = false;
                }
            } else {
                for (int ch = 0; ch < readData.buffer->channelCount(); ch++) {
                    readData.buffer->clear(ch, readData.startPos + beatStart - d->position, beatLength);
                }
                d->tailIsMajor = d->tailIsMinor = false;
            }
        }
        d->position += readData.length;
        return readData.length;
    }

    qint64 MetronomeAudioSource::length() const {
        return std::numeric_limits<qint64>::max();
    }

    qint64 MetronomeAudioSource::nextReadPosition() const {
        return PositionableAudioSource::nextReadPosition();
    }

    void MetronomeAudioSource::setNextReadPosition(qint64 pos) {
        PositionableAudioSource::setNextReadPosition(pos);
    }

    bool MetronomeAudioSource::setMajorBeatSource(PositionableAudioSource *src, bool takeOwnership) {
        Q_D(MetronomeAudioSource);
        QMutexLocker locker(&d->mutex);
        if (isOpen())
            if (!src->open(bufferSize(), sampleRate()))
                return false;
        d->majorBeatSource = src;
        d->takeOwnershipOfMajor = takeOwnership;
        return true;
    }

    PositionableAudioSource *MetronomeAudioSource::majorBeatSource() const {
        Q_D(const MetronomeAudioSource);
        return d->majorBeatSource;
    }

    bool MetronomeAudioSource::setMinorBeatSource(PositionableAudioSource *src, bool takeOwnership) {
        Q_D(MetronomeAudioSource);
        QMutexLocker locker(&d->mutex);
        if (isOpen())
            if (!src->open(bufferSize(), sampleRate()))
                return false;
        d->minorBeatSource = src;
        d->takeOwnershipOfMinor = takeOwnership;
        return true;
    }

    PositionableAudioSource *MetronomeAudioSource::minorBeatSource() const {
        Q_D(const MetronomeAudioSource);
        return d->minorBeatSource;
    }

    void MetronomeAudioSource::setDetector(MetronomeAudioSourceBeatDetector *detector) {
        Q_D(MetronomeAudioSource);
        QMutexLocker locker(&d->mutex);
        d->detector = detector;
        detector->initialize();
    }

    MetronomeAudioSourceBeatDetector *MetronomeAudioSource::detector() const {
        Q_D(const MetronomeAudioSource);
        return d->detector;
    }

    static const double PI = 3.14159265358979323846;
    static const double FACTOR = 3.0 * std::sqrt(3.0) / 8.0;

    static float generateMajor(qint64 i, double sampleRate) {
        return FACTOR * (1 - i / 0.1 / sampleRate) * (std::sin(2.0 * PI * 1200.0 * i / sampleRate) + 0.25 * std::sin(2.0 * PI * 3600 * i / sampleRate));
    }

    static float generateMinor(qint64 i, double sampleRate) {
        return FACTOR * (1 - i / 0.1 / sampleRate) * std::sin(2.0 * PI * 800.0 * i / sampleRate);
    }

    class BuiltInBeatSource : public MemoryAudioSource {
    public:
        using Func = float(*)(qint64, double);
        BuiltInBeatSource(Func f) : f(f), MemoryAudioSource(&buf) {
        }
        ~BuiltInBeatSource() = default;

        bool open(qint64 bufferSize, double sampleRate) override {
            buf.resize(2, 0.1 * sampleRate);
            for (int i = 0; i < buf.sampleCount(); i++) {
                buf.sampleAt(0, i) = buf.sampleAt(1, i) = f(i, sampleRate);
            }
            return MemoryAudioSource::open(bufferSize, sampleRate);
        }

        void close() override {
            MemoryAudioSource::close();
        }

    private:
        AudioBuffer buf;
        Func f;
    };

    PositionableAudioSource *MetronomeAudioSource::builtInMajorBeatSource() {
        return new BuiltInBeatSource(&generateMajor);
    }

    PositionableAudioSource *MetronomeAudioSource::builtInMinorBeatSource() {
        return new BuiltInBeatSource(&generateMinor);
    }

} // talcs