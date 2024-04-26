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

#include <QList>

#include "SineWaveNoteSynthesizer.h"
#include "SineWaveNoteSynthesizer_p.h"

namespace talcs {
    SineWaveNoteSynthesizer::SineWaveNoteSynthesizer() : SineWaveNoteSynthesizer(*new SineWaveNoteSynthesizerPrivate) {

    }

    SineWaveNoteSynthesizer::SineWaveNoteSynthesizer(SineWaveNoteSynthesizerPrivate &d) : AudioSource(d) {

    }

    SineWaveNoteSynthesizer::~SineWaveNoteSynthesizer() {

    }

    bool SineWaveNoteSynthesizer::open(qint64 bufferSize, double sampleRate) {
        Q_D(SineWaveNoteSynthesizer);
        d->rate = std::pow(0.99, 20000.0 / sampleRate);
        return AudioSource::open(bufferSize, sampleRate);
    }

    void SineWaveNoteSynthesizer::close() {
        AudioSource::close();
    }

    qint64 SineWaveNoteSynthesizer::processReading(const AudioSourceReadData &readData) {
        static const double PI = std::acos(-1);
        Q_D(SineWaveNoteSynthesizer);
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
                    double vel = keyInfo.nextVel(d->rate);
                    for (int ch = 0; ch < readData.buffer->channelCount(); ch++) {
                        readData.buffer->sampleAt(ch, readData.startPos + currentPos) += vel * std::sin(2.0 * PI * keyInfo.frequency / sampleRate() * double(keyInfo.x));
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
                d->keys.append({msg.frequency, msg.velocity, .0, 0, true});
            } else {
                if (it != d->keys.end())
                    it->isAttack = false;
            }
            if (msg.position == -1)
                break;
        }
        return readData.length;
    }

    void SineWaveNoteSynthesizer::setDetector(SineWaveNoteSynthesizerDetector *detector) {
        Q_D(SineWaveNoteSynthesizer);
        QMutexLocker locker(&d->mutex);
        d->detector = detector;
    }

    SineWaveNoteSynthesizerDetector *SineWaveNoteSynthesizer::detector() const {
        Q_D(const SineWaveNoteSynthesizer);
        return d->detector;
    }
} // talcs