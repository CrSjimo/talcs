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

#ifndef TALCS_NoteSynthesizer_P_H
#define TALCS_NoteSynthesizer_P_H

#include <cmath>

#include <QList>

#include <TalcsCore/NoteSynthesizer.h>
#include <TalcsCore/private/AudioSource_p.h>

namespace talcs {

    class NoteSynthesizerPrivate : public AudioSourcePrivate {
        Q_DECLARE_PUBLIC(NoteSynthesizer);
    public:
        constexpr static const double INITIAL_RATIO = .00390625;
        QMutex mutex;
        NoteSynthesizerDetector *detector = nullptr;

        struct KeyInfo {
            NoteSynthesizerPrivate *d;
            double frequency;
            double vel;
            double envelop;
            qint64 x;
            double integration;
            enum State {
                Attack,
                Decay,
                Release,
            } state;
            inline double nextVel() {
                double ret = envelop * vel;
                switch (state) {
                    case Attack:
                        if (qFuzzyIsNull(envelop))
                            envelop = INITIAL_RATIO;
                        envelop *= d->attackRate;
                        if (envelop > 1.0) {
                            envelop = 1.0;
                            state = Decay;
                        }
                        if (d->attackTime == 0)
                            ret = vel;
                        break;
                    case Decay:
                        if (envelop > d->decayRatio)
                            envelop *= d->decayRate;
                        break;
                    case Release:
                        envelop *= d->releaseRate;
                        if (envelop < INITIAL_RATIO)
                            envelop = .0;
                }
                x++;
                integration += frequency / d->q_func()->sampleRate() * std::pow(2, d->deltaPitch / 12.0);
                integration = std::fmod(integration, 1.0);
                return ret * d->volume;
            }
        };
        qint64 attackTime = .0;
        qint64 decayTime = .0;
        qint64 releaseTime = .0;

        double attackRate;
        double decayRate;
        double decayRatio = 1.0;
        double releaseRate;

        double deltaPitch = 0.0;
        double volume = 1.0;

        void updateRates();

        QList<KeyInfo> keys;

        struct GenerateSineWave {
            inline double operator()(double integration) {
                static const double PI = std::acos(-1);
                return std::sin(2.0 * PI * integration);
            }
        };

        struct GenerateSquareWave {
            inline double operator()(double integration) {
                double t = std::fmod(integration, 1.0);
                if (t < -0.5) {
                    return 1.0;
                } else if (t < 0.0) {
                    return -1.0;
                } else if (t < 0.5) {
                    return 1.0;
                } else {
                    return -1.0;
                }
            }
        };

        struct GenerateTriangleWave {
            inline double operator()(double integration) {
                double t = std::fmod(integration, 1.0);
                if (t < -0.75) {
                    return 4.0 * (t + 1.0);
                } else if (t < -0.25) {
                    return -4.0 * (t + 0.5);
                } else if (t < 0.25) {
                    return 4.0 * t;
                } else if (t < 0.75) {
                    return -4.0 * (t - 0.5);
                } else {
                    return 4.0 * (t - 1.0);
                }
            }
        };

        struct GenerateSawtoothWave {
            inline double operator()(double integration) {
                double t = std::fmod(integration, 1.0);
                if (t < 0)
                    return 2 * t + 1;
                else
                    return 2 * t - 1;
            }
        };

        NoteSynthesizer::GeneratorFunction generatorFunction = GenerateSineWave();

        void handleNoteMessage(const NoteSynthesizerDetectorMessage::Note &msg);
        void handlePitchMessage(const NoteSynthesizerDetectorMessage::Pitch &msg);
        void handleVolumeMessage(const NoteSynthesizerDetectorMessage::Volume &msg);

    };

}

#endif //TALCS_NoteSynthesizer_P_H
