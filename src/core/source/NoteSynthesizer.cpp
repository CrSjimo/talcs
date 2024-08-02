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

#include <cmath>

#include <QList>

#include "NoteSynthesizer.h"
#include "NoteSynthesizer_p.h"

namespace talcs {

    /**
     * @struct NoteSynthesizerDetectorMessage
     * The message produced by NoteSynthesizerDetector.
     *
     * @var NoteSynthesizerDetectorMessage::position
     * The position of the message relative to the start of interval. If the message is null, this value will be -1.
     *
     * @var NoteSynthesizerDetectorMessage::frequency
     * The frequency (Hz) of the note. If the message is all-notes-off, this value will be 0.0.
     *
     * @var NoteSynthesizerDetectorMessage::velocity
     * The velocity (gain) of the note sound. Ranged between 0.0 and 1.0.
     *
     * @var NoteSynthesizerDetectorMessage::isNoteOn
     * The message is whether note-on of note-off.
     */

    /**
     * @fn NoteSynthesizerDetectorMessage::NoteSynthesizerDetectorMessage(SpecialValueNull)
     * Constructor of null message.
     */

    /**
     * @fn NoteSynthesizerDetectorMessage::NoteSynthesizerDetectorMessage(qint64 position, SpecialValueAllNotesOff)
     * Constructor of all-notes-off message.
     */

    /**
     * @fn NoteSynthesizerDetectorMessage::NoteSynthesizerDetectorMessage(qint64 position, double frequency, bool isNoteOn)
     * Constructor. The velocity is 1.0 by default.
     */

    /**
     * @fn NoteSynthesizerDetectorMessage::NoteSynthesizerDetectorMessage(qint64 position, double frequency, double velocity, bool isNoteOn)
     * Constructor.
     */

    /**
     * @class NoteSynthesizerDetector
     * @brief The detector for NoteSynthesizer
     */

    /**
     * @fn void NoteSynthesizerDetector::detectInterval(qint64 intervalLength)
     * Detects notes in the interval and updates the internal state.
     */

    /**
     * @fn NoteSynthesizerDetectorMessage NoteSynthesizerDetector::nextMessage()
     * Returns the next message within the interval. If there is no more message left, returns null message.
     */

    /**
     * Constructor.
     */
    NoteSynthesizer::NoteSynthesizer() : NoteSynthesizer(*new NoteSynthesizerPrivate) {

    }

    NoteSynthesizer::NoteSynthesizer(NoteSynthesizerPrivate &d) : AudioSource(d) {
        d.updateRates();
    }

    /**
     * Destructor.
     */
    NoteSynthesizer::~NoteSynthesizer() = default;

    bool NoteSynthesizer::open(qint64 bufferSize, double sampleRate) {
        Q_D(NoteSynthesizer);
        return AudioSource::open(bufferSize, sampleRate);
    }

    void NoteSynthesizer::close() {
        AudioSource::close();
    }

    /**
     * Sets the attack time (in sample).
     */
    void NoteSynthesizer::setAttackTime(qint64 t) {
        Q_D(NoteSynthesizer);
        d->attackTime = t;
        d->updateRates();
    }

    /**
     * Gets the attack time (in sample).
     */
    qint64 NoteSynthesizer::attackTime() const {
        Q_D(const NoteSynthesizer);
        return d->attackTime;
    }

    void NoteSynthesizer::setDecayTime(qint64 t) {
        Q_D(NoteSynthesizer);
        d->decayTime = t;
        d->updateRates();
    }
    qint64 NoteSynthesizer::decayTime() const {
        Q_D(const NoteSynthesizer);
        return d->decayTime;
    }
    void NoteSynthesizer::setDecayRatio(double ratio) {
        Q_D(NoteSynthesizer);
        d->decayRatio = ratio;
        d->updateRates();
    }
    double NoteSynthesizer::decayRatio() const {
        Q_D(const NoteSynthesizer);
        return d->decayRatio;
    }

    /**
     * Sets the release time (in sample).
     */
    void NoteSynthesizer::setReleaseTime(qint64 t) {
        Q_D(NoteSynthesizer);
        d->releaseTime = t;
        d->updateRates();
    }

    /**
     * Gets the release time (in sample).
     */
    qint64 NoteSynthesizer::releaseTime() const {
        Q_D(const NoteSynthesizer);
        return d->releaseTime;
    }

    void NoteSynthesizerPrivate::updateRates() {
        attackRate = std::pow(INITIAL_RATIO, -1.0 / attackTime);
        decayRate = std::pow(decayRatio, 1.0 / decayTime);
        releaseRate = std::pow(INITIAL_RATIO / decayRatio, 1.0 / releaseTime);
    }

    /**
     * @enum NoteSynthesizer::Generator
     * Pre-defined generator patterns.
     *
     * @var NoteSynthesizer::Sine
     * Sine wave.
     *
     * @var NoteSynthesizer::Square
     * Square wave.
     *
     * @var NoteSynthesizer::Triangle
     * Triangle wave.
     *
     * @var NoteSynthesizer::Sawtooth
     * Sawtooth wave.
     */

    /**
     * Sets the generator to a pre-defined pattern.
     */
    void NoteSynthesizer::setGenerator(NoteSynthesizer::Generator g) {
        switch (g) {
            case Sine:
                setGenerator(NoteSynthesizerPrivate::GenerateSineWave());
                break;
            case Square:
                setGenerator(NoteSynthesizerPrivate::GenerateSquareWave());
                break;
            case Triangle:
                setGenerator(NoteSynthesizerPrivate::GenerateTriangleWave());
                break;
            case Sawtooth:
                setGenerator(NoteSynthesizerPrivate::GenerateSawtoothWave());
                break;
            default:
                setGenerator(NoteSynthesizerPrivate::GenerateSineWave());
        }
    }

    /**
     * Sets the generator to a custom function.
     * @param g the generator function. The first parameter is frequency (unit: sample^(-1), not Hz) and the second
     * parameter is the position in sample.
     *
     * For example, if this generator generates sine wave, the function will be:
     *
     * @code
     * [](double f, qint64 x) { return std::sin(2.0 * PI * f * double(x)); }
     * @endcode
     */
    void NoteSynthesizer::setGenerator(const NoteSynthesizer::GeneratorFunction &g) {
        Q_D(NoteSynthesizer);
        QMutexLocker locker(&d->mutex);
        d->generatorFunction = g;
    }

    qint64 NoteSynthesizer::processReading(const AudioSourceReadData &readData) {
        Q_D(NoteSynthesizer);
        QMutexLocker locker(&d->mutex);
        for (int ch = 0; ch < readData.buffer->channelCount(); ch++) {
            readData.buffer->clear(ch, readData.startPos, readData.length);
        }
        if (!d->detector)
            return readData.length;
        d->detector->detectInterval(readData.length);
        int currentPos = 0;
        for (auto msg = d->detector->nextMessage(); ; msg = d->detector->nextMessage()) {
            for (;currentPos < (msg.position != -1 ? msg.position : readData.length); currentPos++) {
                for (auto &keyInfo : d->keys) {
                    double vel = keyInfo.nextVel();
                    for (int ch = 0; ch < readData.buffer->channelCount(); ch++) {
                        readData.buffer->sampleAt(ch, readData.startPos + currentPos) += static_cast<float>(vel * d->generatorFunction(keyInfo.frequency / sampleRate(), keyInfo.x));
                    }
                }
                d->keys.erase(std::remove_if(d->keys.begin(), d->keys.end(), [&](const auto &item) {
                    return qFuzzyIsNull(item.envelop);
                }), d->keys.end());
            }
            if (msg.position == -1)
                break;
            if (qFuzzyIsNull(msg.frequency) && msg.messageType == NoteSynthesizerDetectorMessage::NoteOff) { // All notes off
                for (auto &key : d->keys) {
                    key.state = NoteSynthesizerPrivate::KeyInfo::Release;
                }
            } else {
                auto it = std::find_if(d->keys.begin(), d->keys.end(), [&](const auto &item) {
                    return qFuzzyCompare(item.frequency, msg.frequency);
                });
                if (msg.messageType == NoteSynthesizerDetectorMessage::NoteOff) {
                    if (it != d->keys.end())
                        it->state = NoteSynthesizerPrivate::KeyInfo::Release;
                } else {
                    if (it != d->keys.end()) {
                        if (msg.messageType == NoteSynthesizerDetectorMessage::NoteOn || it->state == NoteSynthesizerPrivate::KeyInfo::Release) {
                            it->vel = msg.velocity;
                            it->state = NoteSynthesizerPrivate::KeyInfo::Attack;
                        }
                    } else {
                        d->keys.append({d, msg.frequency, msg.velocity, {}, {}, {}});
                    }
                }
            }
        }
        return readData.length;
    }

    /**
     * Sets the NoteSynthesizerDetector detector.
     */
    void NoteSynthesizer::setDetector(NoteSynthesizerDetector *detector) {
        Q_D(NoteSynthesizer);
        QMutexLocker locker(&d->mutex);
        d->detector = detector;
    }

    /**
     * Gets the NoteSynthesizerDetector detector.
     */
    NoteSynthesizerDetector *NoteSynthesizer::detector() const {
        Q_D(const NoteSynthesizer);
        return d->detector;
    }

    /**
     * Flushes all notes that are being played.
     * @param force if true, the notes will be hard-interrupted, or if false, an all-notes-off message will be applied.
     */
    void NoteSynthesizer::flush(bool force) {
        Q_D(NoteSynthesizer);
        QMutexLocker locker(&d->mutex);
        if (force) {
            d->keys.clear();
        } else {
            for (auto &key : d->keys)
                key.state = NoteSynthesizerPrivate::KeyInfo::Release;
        }

    }
} // talcs