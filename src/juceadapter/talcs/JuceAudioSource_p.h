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

#ifndef TALCS_JUCEAUDIOSOURCE_P_H
#define TALCS_JUCEAUDIOSOURCE_P_H

#include <TalcsCore/AudioDataWrapper.h>
#include <TalcsCore/private/AudioSource_p.h>
#include <TalcsJuceAdapter/JuceAudioSource.h>

#include <juce_audio_basics/juce_audio_basics.h>

namespace talcs {
    class JuceAudioSourcePrivate : public AudioSourcePrivate {
        Q_DECLARE_PUBLIC(JuceAudioSource)
    public:
        juce::AudioSource *src;
        juce::AudioBuffer<float> buf;
        AudioDataWrapper bufWrapper = AudioDataWrapper(nullptr, 0, 0);

    };
}

#endif //TALCS_JUCEAUDIOSOURCE_P_H
