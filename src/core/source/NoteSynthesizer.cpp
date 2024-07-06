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
    NoteSynthesizer::NoteSynthesizer() : NoteSynthesizer(*new NoteSynthesizerPrivate) {

    }

    NoteSynthesizer::NoteSynthesizer(NoteSynthesizerPrivate &d) : AudioSource(d) {

    }

    NoteSynthesizer::~NoteSynthesizer() {

    }

    bool NoteSynthesizer::open(qint64 bufferSize, double sampleRate) {
        Q_D(NoteSynthesizer);
        return AudioSource::open(bufferSize, sampleRate);
    }

    void NoteSynthesizer::close() {
        AudioSource::close();
    }

    void NoteSynthesizer::setAttackRate(double rate) {
        Q_D(NoteSynthesizer);
        d->attackRate = rate;
    }

    double NoteSynthesizer::attackRate() const {
        Q_D(const NoteSynthesizer);
        return d->attackRate;
    }

    void NoteSynthesizer::setReleaseRate(double rate) {
        Q_D(NoteSynthesizer);
        d->releaseRate = rate;
    }

    double NoteSynthesizer::releaseRate() const {
        Q_D(const NoteSynthesizer);
        return d->releaseRate;
    }


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
                Q_UNREACHABLE();
        }
    }

    void NoteSynthesizer::setGenerator(const NoteSynthesizer::GeneratorFunction &g) {
        Q_D(NoteSynthesizer);
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
                    return qFuzzyIsNull(item.vel);
                }), d->keys.end());
            }
            auto it = std::find_if(d->keys.begin(), d->keys.end(), [&](const auto &item) {
                return qFuzzyCompare(item.frequency, msg.frequency);
            });
            if (msg.isNoteOn) {
                d->keys.append({d, msg.frequency, msg.velocity, .0, 0, true});
            } else {
                if (it != d->keys.end())
                    it->isAttack = false;
            }
            if (msg.position == -1)
                break;
        }
        return readData.length;
    }

    void NoteSynthesizer::setDetector(NoteSynthesizerDetector *detector) {
        Q_D(NoteSynthesizer);
        QMutexLocker locker(&d->mutex);
        d->detector = detector;
    }

    NoteSynthesizerDetector *NoteSynthesizer::detector() const {
        Q_D(const NoteSynthesizer);
        return d->detector;
    }
} // talcs