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

#ifndef TALCS_SINEWAVENOTESYNTHESIZER_P_H
#define TALCS_SINEWAVENOTESYNTHESIZER_P_H

#include <QList>

#include <TalcsCore/SineWaveNoteSynthesizer.h>
#include <TalcsCore/private/AudioSource_p.h>

namespace talcs {

    class SineWaveNoteSynthesizerPrivate : public AudioSourcePrivate {
        Q_DECLARE_PUBLIC(SineWaveNoteSynthesizer);
    public:
        QMutex mutex;
        SineWaveNoteSynthesizerDetector *detector = nullptr;
        struct KeyInfo {
            double frequency;
            double velFactor;
            double vel;
            qint64 x;
            bool isAttack;
            inline double nextVel(double rate_) {
                double ret = vel;
                if (isAttack && vel < velFactor) {
                    if (qFuzzyIsNull(vel))
                        vel = .005;
                    vel /= rate_;
                    if (vel > velFactor)
                        vel = velFactor;
                } else {
                    vel *= rate_;
                    if (vel < .005)
                        vel = .0;
                }
                x++;
                return ret;
            }
        };
        double rate = .0;
        QList<KeyInfo> keys;
    };

}

#endif //TALCS_SINEWAVENOTESYNTHESIZER_P_H
