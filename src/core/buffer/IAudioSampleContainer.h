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

#ifndef TALCS_IAUDIOSAMPLECONTAINER_H
#define TALCS_IAUDIOSAMPLECONTAINER_H

#include <TalcsCore/IAudioSampleProvider.h>

namespace talcs {

    class TALCSCORE_EXPORT IAudioSampleContainer : public IAudioSampleProvider {
    public:
        virtual void setSample(int channel, qint64 pos, float value) = 0;
        virtual float *writePointerTo(int channel, qint64 startPos);

        void setSampleRange(int destChannel, qint64 destStartPos, qint64 length, const IAudioSampleProvider &src,
                            int srcChannel, qint64 srcStartPos);
        void setSampleRange(const IAudioSampleProvider &src);

        void addSampleRange(int destChannel, qint64 destStartPos, qint64 length, const IAudioSampleProvider &src,
                            int srcChannel, qint64 srcStartPos, float gain = 1);
        void addSampleRange(const IAudioSampleProvider &src, float gain = 1);

        void gainSampleRange(int destChannel, qint64 destStartPos, qint64 length, float gain);
        void gainSampleRange(int destChannel, float gain);
        void gainSampleRange(float gain);

        void clear(int destChannel, qint64 destStartPos, qint64 length);
        void clear(int destChannel);
        void clear();
    };
    
}

#endif // TALCS_IAUDIOSAMPLECONTAINER_H
