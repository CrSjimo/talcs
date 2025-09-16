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

#include "JucePositionableAudioSource.h"
#include "JucePositionableAudioSource_p.h"

namespace talcs {
    JucePositionableAudioSource::JucePositionableAudioSource(juce::PositionableAudioSource *src) : PositionableAudioSource(*new JucePositionableAudioSourcePrivate) {
        Q_D(JucePositionableAudioSource);
        d->src = src;
    }

    JucePositionableAudioSource::~JucePositionableAudioSource() = default;

    bool JucePositionableAudioSource::open(qint64 bufferSize, double sampleRate) {
        Q_D(JucePositionableAudioSource);
        d->src->prepareToPlay(bufferSize, sampleRate);
        return AudioSource::open(bufferSize, sampleRate);
    }

    void JucePositionableAudioSource::close() {
        Q_D(JucePositionableAudioSource);
        d->src->releaseResources();
        AudioSource::close();
    }

    qint64 JucePositionableAudioSource::processReading(const AudioSourceReadData &readData) {
        Q_D(JucePositionableAudioSource);
        if (readData.length > d->buf.getNumSamples() || readData.buffer->channelCount() > d->buf.getNumChannels()) {
            d->buf.setSize(readData.buffer->channelCount(), readData.length);
            d->bufWrapper.reset(d->buf.getArrayOfWritePointers(), d->buf.getNumChannels(), d->buf.getNumSamples());
        }
        d->src->getNextAudioBlock(juce::AudioSourceChannelInfo(d->buf));
        for (int ch = 0; ch < readData.buffer->channelCount(); ch++) {
            readData.buffer->setSampleRange(ch, readData.startPos, readData.length, d->bufWrapper, ch, 0);
        }
        return qMin(readData.length, d->src->getTotalLength() - d->src->getNextReadPosition());
    }

    qint64 JucePositionableAudioSource::length() const {
        Q_D(const JucePositionableAudioSource);
        return d->src->getTotalLength();
    }

    qint64 JucePositionableAudioSource::nextReadPosition() const {
        Q_D(const JucePositionableAudioSource);
        return d->src->getNextReadPosition();
    }

    void JucePositionableAudioSource::setNextReadPosition(qint64 pos) {
        Q_D(JucePositionableAudioSource);
        d->src->setNextReadPosition(pos);
    }
} // talcs
