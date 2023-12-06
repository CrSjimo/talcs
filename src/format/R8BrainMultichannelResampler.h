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

#ifndef TALCS_R8BRAINMULTICHANNELRESAMPLER_H
#define TALCS_R8BRAINMULTICHANNELRESAMPLER_H

#include <QScopedPointer>

#include <TalcsFormat/TalcsFormatGlobal.h>
#include <TalcsCore/AudioSource.h>

namespace talcs {

    class AudioBuffer;
    class AudioSourceReadData;
    class IAudioSampleProvider;

    class R8BrainMultichannelResamplerPrivate;
    class ChannelResampler;

    class TALCSFORMAT_EXPORT R8BrainMultichannelResampler {
    public:
        explicit R8BrainMultichannelResampler(double ratio, qint64 bufferSize, int channelCount);
        ~R8BrainMultichannelResampler();

        void reset();

        void process(const AudioSourceReadData &readData);

        double ratio() const;
        qint64 bufferSize() const;
        int channelCount() const;

    protected:
        virtual void read(const AudioSourceReadData &readData) = 0;

    private:
        friend class ChannelResampler;
        QScopedPointer<R8BrainMultichannelResamplerPrivate> d;

    };

} // talcs

#endif //TALCS_R8BRAINMULTICHANNELRESAMPLER_H
