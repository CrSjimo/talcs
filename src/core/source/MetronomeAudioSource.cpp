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

#include <TalcsCore/PositionableAudioSource.h>
#include <TalcsCore/AudioBuffer.h>
#include <TalcsCore/MemoryAudioSource.h>

namespace talcs {

    /**
     * @class MetronomeAudioSourceBeatDetector
     * @brief The detector for MetronomeAudioSource
     */

    /**
     * @fn void MetronomeAudioSourceBeatDetector::initialize()
     * Initializes the internal state.
     */

    /**
     * @fn void MetronomeAudioSourceBeatDetector::detectInterval(qint64 intervalStart, qint64 intervalLength)
     * Detects whether beats exists in the interval and updates the internal state.
     */

    /**
     * @fn QPair<qint64, bool> MetronomeAudioSourceBeatDetector::nextBeat()
     * Returns the position (relative to the start of interval) and whether it is a major beat of the next beat within
     * the interval given by previous detectInterval() call. If there is no beat left, then the position should be -1.
     */

    /**
     * @class MetronomeAudioSource
     * @brief The audio source to be used as a metronome
     */

    MetronomeAudioSource::MetronomeAudioSource() : MetronomeAudioSource(*new MetronomeAudioSourcePrivate) {

    }

    MetronomeAudioSource::~MetronomeAudioSource() {
        Q_D(MetronomeAudioSource);
        if (d->takeOwnershipOfMajor)
            delete d->majorBeatSource;
        if (d->takeOwnershipOfMinor)
            delete d->minorBeatSource;
    }

    MetronomeAudioSource::MetronomeAudioSource(MetronomeAudioSourcePrivate &d) : AudioSource(d) {
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
        return AudioSource::open(bufferSize, sampleRate);
    }

    void MetronomeAudioSource::close() {
        Q_D(MetronomeAudioSource);
        QMutexLocker locker(&d->mutex);
        if (d->majorBeatSource)
            d->majorBeatSource->close();
        if (d->minorBeatSource)
            d->minorBeatSource->close();
        AudioSource::close();
    }

    qint64 MetronomeAudioSource::processReading(const AudioSourceReadData &readData) {
        Q_D(MetronomeAudioSource);
        QMutexLocker locker(&d->mutex);
        if (!d->detector) {
            for (int ch = 0; ch < readData.buffer->channelCount(); ch++) {
                readData.buffer->clear(ch, readData.startPos, readData.length);
            }
            return readData.length;
        }
        d->detector->detectInterval(readData.length);
        auto beat = d->detector->nextMessage();
        qint64 tailLength = beat.position != -1 ? beat.position : readData.length;
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
        while (beat.position != -1) {
            qint64 beatStart = beat.position;
            bool isMajor = beat.isMajor;
            beat = d->detector->nextMessage();
            qint64 beatLength = beat.position != -1 ? beat.position - beatStart : readData.length - beatStart;
            auto beatSource = isMajor ? d->majorBeatSource : d->minorBeatSource;
            d->tailIsMajor = isMajor;
            d->tailIsMinor = !isMajor;
            if (beatSource) {
                beatSource->setNextReadPosition(0);
                qint64 beatReadLength = beatSource->read({readData.buffer, readData.startPos + beatStart, beatLength, readData.silentFlags});
                if (beatReadLength < beatLength) {
                    for (int ch = 0; ch < readData.buffer->channelCount(); ch++) {
                        readData.buffer->clear(ch, readData.startPos + beatStart, beatLength - beatReadLength);
                    }
                    d->tailIsMajor = d->tailIsMinor = false;
                }
            } else {
                for (int ch = 0; ch < readData.buffer->channelCount(); ch++) {
                    readData.buffer->clear(ch, readData.startPos + beatStart, beatLength);
                }
                d->tailIsMajor = d->tailIsMinor = false;
            }
        }
        return readData.length;
    }

    /**
     * Sets the source for major beat.
     * @return true if successful
     */
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

    /**
     * Gets the source for major beat.
     */
    PositionableAudioSource *MetronomeAudioSource::majorBeatSource() const {
        Q_D(const MetronomeAudioSource);
        return d->majorBeatSource;
    }

    /**
     * Sets the source for minor beat.
     * @return true if successful
     */
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

    /**
     * Gets the source for minor beat.
     */
    PositionableAudioSource *MetronomeAudioSource::minorBeatSource() const {
        Q_D(const MetronomeAudioSource);
        return d->minorBeatSource;
    }

    /**
     * Sets the detector.
     * @see MetronomeAudioSourceBeatDetector
     */
    void MetronomeAudioSource::setDetector(MetronomeAudioSourceDetector *detector) {
        Q_D(MetronomeAudioSource);
        QMutexLocker locker(&d->mutex);
        d->detector = detector;
    }

    /**
     * Gets the detector.
     * @see MetronomeAudioSourceBeatDetector
     */
    MetronomeAudioSourceDetector *MetronomeAudioSource::detector() const {
        Q_D(const MetronomeAudioSource);
        return d->detector;
    }

    static const double PI = std::acos(-1);
    static const double FACTOR = 3.0 * std::sqrt(3.0) / 8.0;

    static float generateMajor(qint64 i, double sampleRate) {
        return static_cast<float>(FACTOR * (1 - static_cast<double>(i) / 0.1 / sampleRate) * (std::sin(2.0 * PI * 1200.0 * static_cast<double>(i) / sampleRate) + 0.25 * std::sin(2.0 * PI * 3600 * static_cast<double>(i) / sampleRate)));
    }

    static float generateMinor(qint64 i, double sampleRate) {
        return static_cast<float>(FACTOR * (1 - static_cast<double>(i) / 0.1 / sampleRate) * std::sin(2.0 * PI * 800.0 * static_cast<double>(i) / sampleRate));
    }

    class BuiltInBeatSource : public MemoryAudioSource {
    public:
        using Func = float(*)(qint64, double);
        explicit BuiltInBeatSource(Func f) : f(f), MemoryAudioSource(&buf) {
        }
        ~BuiltInBeatSource() override = default;

        bool open(qint64 bufferSize, double sampleRate) override {
            buf.resize(2, static_cast<qint64>(0.1 * sampleRate));
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

    /**
     * Creates a built-in major beat source. The returned object is managed by caller.
     */
    PositionableAudioSource *MetronomeAudioSource::builtInMajorBeatSource() {
        return new BuiltInBeatSource(&generateMajor);
    }

    /**
     * Creates a built-in minor beat source. The returned object is managed by caller.
     */
    PositionableAudioSource *MetronomeAudioSource::builtInMinorBeatSource() {
        return new BuiltInBeatSource(&generateMinor);
    }

} // talcs