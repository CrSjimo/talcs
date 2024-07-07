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
        QMutex mutex;
        NoteSynthesizerDetector *detector = nullptr;

        struct KeyInfo {
            NoteSynthesizerPrivate *d;
            double frequency;
            double velFactor;
            double vel;
            qint64 x;
            bool isAttack;
            inline double nextVel() {
                double ret = vel;
                if (isAttack && vel < velFactor) {
                    if (qFuzzyIsNull(vel))
                        vel = .005;
                    vel /= d->attackRate;
                    if (vel > velFactor)
                        vel = velFactor;
                } else if (!isAttack) {
                    vel *= d->releaseRate;
                    if (vel < .005)
                        vel = .0;
                }
                x++;
                return ret;
            }
        };
        double attackRate = .005;
        double releaseRate = .0;
        QList<KeyInfo> keys;

        struct GenerateSineWave {
            inline double operator()(double f, qint64 x) {
                static const double PI = std::acos(-1);
                return std::sin(2.0 * PI * f * double(x));
            }
        };

        struct GenerateSquareWave {
            inline double operator()(double f, qint64 x) {
                double period = 1.0 / f;
                double t = fmod(x, period);
                return (t < period / 2) ? 1.0 : -1.0;
            }
        };

        struct GenerateTriangleWave {
            inline double operator()(double f, qint64 x) {
                double period = 1.0 / f;
                double t = fmod(x, period);
                double normalized_t = t / period;

                if (normalized_t < 0.25) {
                    return 4.0 * normalized_t;
                } else if (normalized_t < 0.75) {
                    return 2.0 - 4.0 * normalized_t;
                } else {
                    return -4.0 + 4.0 * normalized_t;
                }
            }
        };

        struct GenerateSawtoothWave {
            inline double operator()(double f, qint64 x) {
                double period = 1.0 / f;
                double t = fmod(x, period);
                return 2.0 * (t / period) - 1.0;
            }
        };

        NoteSynthesizer::GeneratorFunction generatorFunction = GenerateSineWave();

    };

}

#endif //TALCS_NoteSynthesizer_P_H
