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

#include "JuceAudioSource.h"
#include "JuceAudioSource_p.h"

namespace talcs {
    JuceAudioSource::JuceAudioSource(juce::AudioSource *src) : AudioSource(*new JuceAudioSourcePrivate) {
        Q_D(JuceAudioSource);
        d->src = src;
    }

    JuceAudioSource::~JuceAudioSource() = default;

    bool JuceAudioSource::open(qint64 bufferSize, double sampleRate) {
        Q_D(JuceAudioSource);
        d->src->prepareToPlay(bufferSize, sampleRate);
        return AudioStreamBase::open(bufferSize, sampleRate);
    }

    void JuceAudioSource::close() {
        Q_D(JuceAudioSource);
        d->src->releaseResources();
        AudioStreamBase::close();
    }

    qint64 JuceAudioSource::read(const AudioSourceReadData &readData) {
        Q_D(JuceAudioSource);
        if (readData.length > d->buf.getNumSamples() || readData.buffer->channelCount() > d->buf.getNumChannels()) {
            d->buf.setSize(readData.buffer->channelCount(), readData.length);
            d->bufWrapper.reset(d->buf.getArrayOfWritePointers(), d->buf.getNumChannels(), d->buf.getNumSamples());
        }
        d->src->getNextAudioBlock(juce::AudioSourceChannelInfo(d->buf));
        for (int ch = 0; ch < readData.buffer->channelCount(); ch++) {
            readData.buffer->setSampleRange(ch, readData.startPos, readData.length, d->bufWrapper, ch, 0);
        }
        return readData.length;
    }
} // talcs