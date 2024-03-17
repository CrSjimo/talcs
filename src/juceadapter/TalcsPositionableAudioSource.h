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

#ifndef TALCS_TALCSPOSITIONABLEAUDIOSOURCE_H
#define TALCS_TALCSPOSITIONABLEAUDIOSOURCE_H

#include <TalcsJuceAdapter/TalcsJuceAdapterGlobal.h>

#include <juce_audio_basics/juce_audio_basics.h>

#include <QScopedPointer>

namespace talcs {

    class PositionableAudioSource;

    class TalcsPositionableAudioSourcePrivate;

    class TALCSJUCEADAPTER_EXPORT TalcsPositionableAudioSource : public juce::PositionableAudioSource {
        Q_DECLARE_PRIVATE(TalcsPositionableAudioSource)
    public:
        explicit TalcsPositionableAudioSource(talcs::PositionableAudioSource *src, bool takeOwnership = false);
        ~TalcsPositionableAudioSource() override;

        void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
        void getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) override;
        void releaseResources() override;

        juce::int64 getTotalLength() const override;
        juce::int64 getNextReadPosition() const override;
        void setNextReadPosition(juce::int64 newPosition) override;

        bool isLooping() const override;
        void setLooping(bool shouldLoop) override;

    protected:
        TalcsPositionableAudioSource(TalcsPositionableAudioSourcePrivate &d);

    private:
        QScopedPointer<TalcsPositionableAudioSourcePrivate> d_ptr;
    };
}

#endif //TALCS_TALCSPOSITIONABLEAUDIOSOURCE_H
