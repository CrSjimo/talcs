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

#ifndef TALCS_IAUDIOSAMPLEPROVIDER_H
#define TALCS_IAUDIOSAMPLEPROVIDER_H

#include <QPair>

#include <TalcsCore/TalcsCoreGlobal.h>

namespace talcs {

    class TALCSCORE_EXPORT IAudioSampleProvider {
    public:
        virtual ~IAudioSampleProvider() = default;

        virtual float sample(int channel, qint64 pos) const = 0;
        virtual bool isContinuous() const;
        virtual const float *readPointerTo(int channel, qint64 startPos) const;

        virtual int channelCount() const = 0;
        virtual qint64 sampleCount() const = 0;

        float magnitude(int channel, qint64 startPos, qint64 length) const;
        float magnitude(int channel) const;

        QPair<float, float> findMinMax(int channel, qint64 startPos, qint64 length) const;
        QPair<float, float> findMinMax(int channel) const;

        float rms(int channel, qint64 startPos, qint64 length) const;
        float rms(int channel) const;
    };

}

#endif // TALCS_IAUDIOSAMPLEPROVIDER_H
