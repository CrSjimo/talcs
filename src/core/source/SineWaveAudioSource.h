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

#ifndef TALCS_SINEWAVEAUDIOSOURCE_H
#define TALCS_SINEWAVEAUDIOSOURCE_H

#include <functional>

#include <TalcsCore/PositionableAudioSource.h>

namespace talcs {

    class SineWaveAudioSourcePrivate;

    class TALCSCORE_EXPORT SineWaveAudioSource : public PositionableAudioSource {
        Q_DECLARE_PRIVATE(SineWaveAudioSource)
    public:
        explicit SineWaveAudioSource(double frequency);
        explicit SineWaveAudioSource(const std::function<double(qint64)> &frequencyIntegration);
        ~SineWaveAudioSource() override = default;

        bool open(qint64 bufferSize, double sampleRate) override;
        qint64 length() const override;

        void setFrequency(double frequency);
        void setFrequency(const std::function<double(qint64)> &frequencyIntegration);
        std::function<double(qint64)> frequency() const;

        void setNextReadPosition(qint64 pos) override;

    protected:
        qint64 processReading(const AudioSourceReadData &readData) override;

    };
    
}

#endif // TALCS_SINEWAVEAUDIOSOURCE_H
