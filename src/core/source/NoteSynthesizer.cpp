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
     * @brief The message produced by NoteSynthesizerDetector.
     */

    /**
     * @enum NoteSynthesizerDetectorMessage::SpecialValueNull
     * Special null message.
     *
     * @var NoteSynthesizerDetectorMessage::Null
     * Null.
     */

    /**
     * @enum NoteSynthesizerDetectorMessage::SpecialValueAllNotesOff
     * Special all-notes-off message.
     *
     * @var NoteSynthesizerDetectorMessage::AllNotesOff
     * All notes off.
     */

    /**
     * @enum NoteSynthesizerDetectorMessage::NoteMessageType
     * Types of the note message.
     *
     * @var NoteSynthesizerDetectorMessage::NoteOff
     * Note off.
     *
     * @var NoteSynthesizerDetectorMessage::NoteOn
     * Note on.
     *
     * @var NoteSynthesizerDetectorMessage::NoteOnIfNotPlaying
     * Note on if the note is not playing.
     */

    /**
     * @struct NoteSynthesizerDetectorMessage::Note
     * @brief Struct of a note message.
     */

    /**
     * @fn NoteSynthesizerDetectorMessage::Note::Note(SpecialValueAllNotesOff allNotesOff)
     * Creates a special all-notes-off message.
     */

    /**
     * @fn NoteSynthesizerDetectorMessage::Note::Note(double frequency, NoteMessageType messageType)
     * @overload
     * Creates a note message with velocity set to 1.0.
     */

    /**
     * @fn NoteSynthesizerDetectorMessage::Note::Note(double frequency, double velocity, NoteMessageType messageType)
     * Creates a note message.
     */

    /**
     * @var NoteSynthesizerDetectorMessage::Note::frequency
     * Frequency.
     */

    /**
     * @var NoteSynthesizerDetectorMessage::Note::velocity
     * Velocity.
     */

    /**
     * @var NoteSynthesizerDetectorMessage::Note::messageType
     * Message type.
     */

    /**
     * @struct NoteSynthesizerDetectorMessage::Pitch
     * @brief Struct of a pitch message.
     */

    /**
     * @var NoteSynthesizerDetectorMessage::Pitch::deltaPitch
     * Delta pitch in semitone.
     */

    /**
     * @struct NoteSynthesizerDetectorMessage::Volume
     * @brief Struct of a volume message.
     */

    /**
     * @var NoteSynthesizerDetectorMessage::Volume::volume
     * The volume. Will be multiplied to the actual gain.
     */

    /**
     * @fn NoteSynthesizerDetectorMessage::NoteSynthesizerDetectorMessage(SpecialValueNull null)
     * Creates a null message.
     */

    /**
     * @fn NoteSynthesizerDetectorMessage::NoteSynthesizerDetectorMessage(qint64 position, const Note &note)
     * Creates a note message.
     */

    /**
     * @fn NoteSynthesizerDetectorMessage::NoteSynthesizerDetectorMessage(qint64 position, const Pitch &pitch)
     * Creates a pitch message.
     */

    /**
     * @fn NoteSynthesizerDetectorMessage::NoteSynthesizerDetectorMessage(qint64 position, const Volume &volume)
     * Creates a volume message.
     */

    /**
     * @fn bool NoteSynthesizerDetectorMessage::isNull() const
     * Gets whether the message is null.
     */

    /**
     * @var NoteSynthesizerDetectorMessage::position
     * Position of the message. For null message, it is -1.
     */

    /**
     * @var NoteSynthesizerDetectorMessage::messageType;
     * Message type.
     */

    /**
     * @var NoteSynthesizerDetectorMessage::note
     * The note message struct.
     */

    /**
     * @var NoteSynthesizerDetectorMessage::pitch
     * The pitch message struct.
     */

    /**
     * @var NoteSynthesizerDetectorMessage::volume
     * The volume message struct.
     */

    /**
     * @class NoteSynthesizerDetector
     * @brief The detector for NoteSynthesizer
     *
     * @see MetronomeAudioSourceDetector
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
     * @class NoteSynthesizerConfig
     * @brief Configuration of NoteSynthesizer
     */

    /**
     * Constructor.
     */
    NoteSynthesizerConfig::NoteSynthesizerConfig() : d(new NoteSynthesizerConfigData) {
        d->updateRates();
    }

    NoteSynthesizerConfig::NoteSynthesizerConfig(const NoteSynthesizerConfig &o) = default;

    /**
     * Destructor.
     */
    NoteSynthesizerConfig::~NoteSynthesizerConfig() = default;

    NoteSynthesizerConfig &NoteSynthesizerConfig::operator=(const NoteSynthesizerConfig &o) = default;

    /**
     * Sets the attack time (in sample).
     */
    void NoteSynthesizerConfig::setAttackTime(qint64 t) {
        d->attackTime = t;
        d->updateRates();
    }

    /**
     * Gets the attack time (in sample).
     */
    qint64 NoteSynthesizerConfig::attackTime() const {
        return d->attackTime;
    }

    /**
     * Sets the decay time (in sample).
     */
    void NoteSynthesizerConfig::setDecayTime(qint64 t) {
        d->decayTime = t;
        d->updateRates();
    }

    /**
     * Gets the decay time (in sample).
     */
    qint64 NoteSynthesizerConfig::decayTime() const {
        return d->decayTime;
    }

    /**
     * Sets the decay ratio.
     */
    void NoteSynthesizerConfig::setDecayRatio(double ratio) {
        d->decayRatio = ratio;
        d->updateRates();
    }

    /**
     * Gets thedecay ratio.
     */
    double NoteSynthesizerConfig::decayRatio() const {
        return d->decayRatio;
    }

    /**
     * Sets the release time (in sample).
     */
    void NoteSynthesizerConfig::setReleaseTime(qint64 t) {
        d->releaseTime = t;
        d->updateRates();
    }

    /**
     * Gets the release time (in sample).
     */
    qint64 NoteSynthesizerConfig::releaseTime() const {
        return d->releaseTime;
    }

    /**
     * Sets the generator to a pre-defined pattern.
     */
    void NoteSynthesizerConfig::setGenerator(NoteSynthesizer::Generator g) {
        switch (g) {
            case NoteSynthesizer::Generator::Sine:
                setGenerator(NoteSynthesizerPrivate::GenerateSineWave());
                break;
            case NoteSynthesizer::Generator::Square:
                setGenerator(NoteSynthesizerPrivate::GenerateSquareWave());
                break;
            case NoteSynthesizer::Generator::Triangle:
                setGenerator(NoteSynthesizerPrivate::GenerateTriangleWave());
                break;
            case NoteSynthesizer::Generator::Sawtooth:
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
    void NoteSynthesizerConfig::setGenerator(const NoteSynthesizer::GeneratorFunction &g) {
        d->generatorFunction = g;
    }

    /**
     * Sets the amplitude.
     */
    void NoteSynthesizerConfig::setAmplitude(float amplitude) {
        d->amplitude = amplitude;
    }

    /**
     * Gets the amplitude.
     */
    float NoteSynthesizerConfig::amplitude() const {
        return d->amplitude;
    }

    /**
     * @class NoteSynthesizer
     * @brief A synthesizer of notes.
     */

    /**
     * Constructor.
     */
    NoteSynthesizer::NoteSynthesizer() : NoteSynthesizer(*new NoteSynthesizerPrivate) {

    }

    NoteSynthesizer::NoteSynthesizer(NoteSynthesizerPrivate &d) : AudioSource(d) {

    }

    /**
     * Destructor.
     */
    NoteSynthesizer::~NoteSynthesizer() = default;

    bool NoteSynthesizer::open(qint64 bufferSize, double sampleRate) {
        Q_D(NoteSynthesizer);
        AudioSource::close();
        flush(true);
        return AudioSource::open(bufferSize, sampleRate);
    }

    void NoteSynthesizer::close() {
        flush(true);
        AudioSource::close();
    }

    /**
     * Sets the attack time (in sample).
     *
     * @see NoteSynthesizerConfig::setAttackTime()
     */
    void NoteSynthesizer::setAttackTime(qint64 t) {
        Q_D(NoteSynthesizer);
        QMutexLocker locker(&d->mutex);
        d->config.setAttackTime(t);
    }

    /**
     * Gets the attack time (in sample).
     *
     * @see NoteSynthesizerConfig::attackTime()
     */
    qint64 NoteSynthesizer::attackTime() const {
        Q_D(const NoteSynthesizer);
        return d->config.attackTime();
    }

    /**
     * Sets the decay time (in sample).
     *
     * @see NoteSynthesizerConfig::setDecayTime()
     */
    void NoteSynthesizer::setDecayTime(qint64 t) {
        Q_D(NoteSynthesizer);
        QMutexLocker locker(&d->mutex);
        d->config.setDecayTime(t);
    }

    /**
     * Gets the decay time (in sample).
     *
     * @see NoteSynthesizerConfig::decayTime()
     */
    qint64 NoteSynthesizer::decayTime() const {
        Q_D(const NoteSynthesizer);
        return d->config.decayTime();
    }

    /**
     * Sets the decay ratio.
     *
     * @see NoteSynthesizerConfig::setDecayRatio()
     */
    void NoteSynthesizer::setDecayRatio(double ratio) {
        Q_D(NoteSynthesizer);
        QMutexLocker locker(&d->mutex);
        d->config.setDecayRatio(ratio);
    }

    /**
     * Gets the decay ratio.
     *
     * @see NoteSynthesizerConfig::decayRatio()
     */
    double NoteSynthesizer::decayRatio() const {
        Q_D(const NoteSynthesizer);
        return d->config.decayRatio();
    }

    /**
     * Sets the release time (in sample).
     *
     * @see NoteSynthesizerConfig::setReleaseTime()
     */
    void NoteSynthesizer::setReleaseTime(qint64 t) {
        Q_D(NoteSynthesizer);
        QMutexLocker locker(&d->mutex);
        d->config.setReleaseTime(t);
    }

    /**
     * Gets the release time (in sample).
     *
     * @see NoteSynthesizerConfig::releaseTime()
     */
    qint64 NoteSynthesizer::releaseTime() const {
        Q_D(const NoteSynthesizer);
        return d->config.releaseTime();
    }

    void NoteSynthesizerConfigData::updateRates() {
        attackRate = std::pow(INITIAL_RATIO, -1.0 / static_cast<double>(attackTime));
        decayRate = std::pow(decayRatio, 1.0 / static_cast<double>(decayTime));
        releaseRate = std::pow(INITIAL_RATIO / decayRatio, 1.0 / static_cast<double>(releaseTime));
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
     *
     * @see NoteSynthesizerConfig::setGenerator
     */
    void NoteSynthesizer::setGenerator(NoteSynthesizer::Generator g) {
        Q_D(NoteSynthesizer);
        QMutexLocker locker(&d->mutex);
        d->config.setGenerator(g);
    }

    /**
     * Sets the generator to a custom function.
     *
     * @see NoteSynthesizerConfig::setGenerator
     */
    void NoteSynthesizer::setGenerator(const NoteSynthesizer::GeneratorFunction &g) {
        Q_D(NoteSynthesizer);
        QMutexLocker locker(&d->mutex);
        d->config.setGenerator(g);
    }

    /**
     * Sets the amplitude.
     *
     * @see NoteSynthesizerConfig::setAmplitude()
     */
    void NoteSynthesizer::setAmplitude(float amplitude) {
        Q_D(NoteSynthesizer);
        QMutexLocker locker(&d->mutex);
        d->config.setAmplitude(amplitude);
    }

    /**
     * Gets the amplitude.
     *
     * @see NoteSynthesizerConfig::amplitude()
     */
    float NoteSynthesizer::amplitude() const {
        Q_D(const NoteSynthesizer);
        return d->config.amplitude();
    }

    /**
     * Sets the configuration.
     */
    void NoteSynthesizer::setConfig(const NoteSynthesizerConfig &config) {
        Q_D(NoteSynthesizer);
        QMutexLocker locker(&d->mutex);
        d->config = config;
    }

    /**
     * Gets the configuration.
     */
    NoteSynthesizerConfig NoteSynthesizer::config() const {
        Q_D(const NoteSynthesizer);
        return d->config;
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
                    if (readData.silentFlags == -1)
                        continue;
                    auto generatedSample = static_cast<float>(vel * d->generate(keyInfo.integration));
                    for (int ch = 0; ch < readData.buffer->channelCount(); ch++) {
                        float oldVal = readData.buffer->sample(ch, readData.startPos + currentPos);
                        readData.buffer->setSample(ch, readData.startPos + currentPos, oldVal + generatedSample);
                    }
                }
                d->keys.erase(std::remove_if(d->keys.begin(), d->keys.end(), [&](const auto &item) {
                    return qFuzzyIsNull(item.envelop);
                }), d->keys.end());
            }
            if (msg.isNull())
                break;

            switch (msg.messageType) {
                case NoteSynthesizerDetectorMessage::NoteMessage:
                    d->handleNoteMessage(msg.note);
                    break;
                case NoteSynthesizerDetectorMessage::PitchMessage:
                    d->handlePitchMessage(msg.pitch);
                    break;
                case NoteSynthesizerDetectorMessage::VolumeMessage:
                    d->handleVolumeMessage(msg.volume);
                    break;
            }

        }
        return readData.length;
    }

    void NoteSynthesizerPrivate::handleNoteMessage(const NoteSynthesizerDetectorMessage::Note &msg) {
        if (qFuzzyIsNull(msg.frequency) && msg.messageType == NoteSynthesizerDetectorMessage::NoteOff) { // All notes off
            for (auto &key : keys) {
                key.state = NoteSynthesizerPrivate::KeyInfo::Release;
            }
        } else {
            auto it = std::find_if(keys.begin(), keys.end(), [&](const auto &item) {
                return qFuzzyCompare(item.frequency, msg.frequency);
            });
            if (msg.messageType == NoteSynthesizerDetectorMessage::NoteOff) {
                if (it != keys.end())
                    it->state = NoteSynthesizerPrivate::KeyInfo::Release;
            } else {
                if (it != keys.end()) {
                    if (msg.messageType == NoteSynthesizerDetectorMessage::NoteOn || it->state == NoteSynthesizerPrivate::KeyInfo::Release) {
                        it->vel = msg.velocity;
                        it->state = NoteSynthesizerPrivate::KeyInfo::Attack;
                    }
                } else {
                    keys.append({this, msg.frequency, msg.velocity, {}, {}, {}});
                }
            }
        }
    }

    void NoteSynthesizerPrivate::handlePitchMessage(const NoteSynthesizerDetectorMessage::Pitch &msg) {
        deltaPitch = msg.deltaPitch;
    }

    void NoteSynthesizerPrivate::handleVolumeMessage(const NoteSynthesizerDetectorMessage::Volume &msg) {
        volume = msg.volume;
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