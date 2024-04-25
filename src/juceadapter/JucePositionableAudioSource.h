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

#ifndef TALCS_JUCEPOSITIONABLEAUDIOSOURCE_H
#define TALCS_JUCEPOSITIONABLEAUDIOSOURCE_H

namespace juce {
    class PositionableAudioSource;
}

#include <TalcsCore/PositionableAudioSource.h>
#include <TalcsJuceAdapter/TalcsJuceAdapterGlobal.h>

namespace talcs {

    class JucePositionableAudioSourcePrivate;

    class TALCSJUCEADAPTER_EXPORT JucePositionableAudioSource : public PositionableAudioSource {
        Q_DECLARE_PRIVATE(JucePositionableAudioSource)
    public:
        explicit JucePositionableAudioSource(juce::PositionableAudioSource *src);
        ~JucePositionableAudioSource() override;

        bool open(qint64 bufferSize, double sampleRate) override;
        void close() override;

        qint64 length() const override;
        qint64 nextReadPosition() const override;
        void setNextReadPosition(qint64 pos) override;

    protected:
        qint64 processReading(const AudioSourceReadData &readData) override;

    };

} // talcs

#endif //TALCS_JUCEPOSITIONABLEAUDIOSOURCE_H
