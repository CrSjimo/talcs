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

#ifndef TALCS_INTERLEAVEDAUDIODATAWRAPPER_H
#define TALCS_INTERLEAVEDAUDIODATAWRAPPER_H

#include <QScopedPointer>

#include <TalcsCore/IAudioSampleContainer.h>

namespace talcs {
    
    class InterleavedAudioDataWrapperPrivate;

    class TALCSCORE_EXPORT InterleavedAudioDataWrapper : public IAudioSampleContainer {
    public:
        InterleavedAudioDataWrapper(float *data, int channelCount, qint64 sampleCount);
        ~InterleavedAudioDataWrapper() override;

        float &sampleAt(int channel, qint64 pos) override;
        float constSampleAt(int channel, qint64 pos) const override;
        int channelCount() const override;
        qint64 sampleCount() const override;

        float *writePointerTo(int channel, qint64 startPos) override;
        const float *readPointerTo(int channel, qint64 startPos) const override;

        bool isContinuous() const override;

        float *data() const;
        void reset(float *data, int channelCount, qint64 sampleCount);

        InterleavedAudioDataWrapper(const InterleavedAudioDataWrapper &other);
        InterleavedAudioDataWrapper(InterleavedAudioDataWrapper &&other);
        InterleavedAudioDataWrapper &operator=(const InterleavedAudioDataWrapper &other);
        InterleavedAudioDataWrapper &operator=(InterleavedAudioDataWrapper &&other);

        bool isDuplicatable() const override;

        DuplicatableObject *duplicate() const override;

    private:
        QScopedPointer<InterleavedAudioDataWrapperPrivate> d;
    };

}

#endif // TALCS_INTERLEAVEDAUDIODATAWRAPPER_H
