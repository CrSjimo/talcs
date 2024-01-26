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

#ifndef TALCS_AUDIOBUFFER_H
#define TALCS_AUDIOBUFFER_H

#include <QVector>

#include <TalcsCore/IAudioSampleContainer.h>

namespace talcs {

    class TALCSCORE_EXPORT AudioBuffer : public IAudioSampleContainer {
    public:
        AudioBuffer();
        AudioBuffer(int channelCount, qint64 sampleCount);

        float &sampleAt(int channel, qint64 pos) override;
        float constSampleAt(int channel, qint64 pos) const override;

        float *writePointerTo(int channel, qint64 startPos) override;
        const float *readPointerTo(int channel, qint64 startPos) const override;

        int channelCount() const override;
        qint64 sampleCount() const override;

        bool isContinuous() const override;

        void resize(int newChannelCount = -1, qint64 newSampleCount = -1);

        float *data(int channel);
        float const *constData(int channel) const;

        AudioBuffer slice(int startChannelIndex, qint64 startSampleCount, int channelSize = -1,
                          qint64 length = -1) const;

        static AudioBuffer from(const IAudioSampleProvider &src);

        bool isDuplicatable() const override;

        DuplicatableObject *duplicate() const override;

    private:
        QVector<QVector<float>> m_buffer;
    };
}

#endif // TALCS_AUDIOBUFFER_H
