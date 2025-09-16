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

#include "TalcsAudioSource.h"
#include "TalcsAudioSource_p.h"

#include <TalcsCore/AudioSource.h>

namespace talcs {
    TalcsAudioSource::TalcsAudioSource(talcs::AudioSource *src, bool takeOwnership) : TalcsAudioSource(*new TalcsAudioSourcePrivate) {
        Q_D(TalcsAudioSource);
        d->src.reset(src, takeOwnership);
    }

    TalcsAudioSource::~TalcsAudioSource() = default;

    void TalcsAudioSource::prepareToPlay(int samplesPerBlockExpected, double sampleRate) {
        Q_D(TalcsAudioSource);
        d->src->open(samplesPerBlockExpected, sampleRate);
    }

    void TalcsAudioSource::getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) {
        Q_D(TalcsAudioSource);
        d->wrapper.reset(bufferToFill.buffer->getArrayOfWritePointers(), bufferToFill.buffer->getNumChannels(), bufferToFill.numSamples, bufferToFill.startSample);
        d->src->read(&d->wrapper);
    }

    void TalcsAudioSource::releaseResources() {
        Q_D(TalcsAudioSource);
        d->src->close();
    }

    TalcsAudioSource::TalcsAudioSource(TalcsAudioSourcePrivate &d) : d_ptr(&d) {
        d.q_ptr = this;
    }
} // talcs
