/******************************************************************************
 * Copyright (c) 2023 CrSjimo                                                 *
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

#ifndef TALCS_AUDIOFORMATINPUTSOURCE_P_H
#define TALCS_AUDIOFORMATINPUTSOURCE_P_H

#include <TalcsCore/private/PositionableAudioSource_p.h>
#include <TalcsFormat/AudioFormatInputSource.h>

#include "MultichannelAudioResampler.h"

#include <QMutex>
#include <QVector>

namespace talcs {
    class AudioFormatIO;

    class AudioFormatInputSourcePrivate : public PositionableAudioSourcePrivate {
        Q_DECLARE_PUBLIC(AudioFormatInputSource);
    public:
        AudioFormatIO *io = nullptr;
        bool takeOwnership;
        double ratio = 0;

        class AudioFormatInputResampler : public MultichannelAudioResampler {
        public:
            AudioFormatInputResampler(double ratio, qint64 bufferSize, int channelCount, AudioFormatInputSourcePrivate *d);
            void read(const talcs::AudioSourceReadData &readData) override;
            AudioFormatInputSourcePrivate *d;
            QVector<float> tmpBuf;
        };

        AudioFormatInputResampler *resampler = nullptr;

        qint64 inPosition = 0;

        bool doStereoize = true;

        QMutex mutex;

    };
}

#endif // TALCS_AUDIOFORMATINPUTSOURCE_P_H
