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

#ifndef TALCS_JUCEAUDIOSOURCE_H
#define TALCS_JUCEAUDIOSOURCE_H

namespace juce {
    class AudioSource;
}

#include <TalcsCore/AudioSource.h>
#include <TalcsJuceAdapter/TalcsJuceAdapterGlobal.h>

namespace talcs {

    class JuceAudioSourcePrivate;

    class TALCSJUCEADAPTER_EXPORT JuceAudioSource : public AudioSource {
        Q_DECLARE_PRIVATE(JuceAudioSource)
    public:
        JuceAudioSource(juce::AudioSource *src);
        ~JuceAudioSource() override;

        bool open(qint64 bufferSize, double sampleRate) override;
        void close() override;
        qint64 read(const AudioSourceReadData &readData) override;
    };

} // talcs

#endif //TALCS_JUCEAUDIOSOURCE_H
