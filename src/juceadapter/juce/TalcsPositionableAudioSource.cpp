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

#include "TalcsPositionableAudioSource.h"
#include "TalcsPositionableAudioSource_p.h"

#include <TalcsCore/PositionableAudioSource.h>

namespace talcs {
    TalcsPositionableAudioSource::TalcsPositionableAudioSource(talcs::PositionableAudioSource *src, bool takeOwnership) : TalcsPositionableAudioSource(*new TalcsPositionableAudioSourcePrivate) {
        Q_D(TalcsPositionableAudioSource);
        d->src = src;
        d->takeOwnership = takeOwnership;
    }

    TalcsPositionableAudioSource::~TalcsPositionableAudioSource() {
        Q_D(TalcsPositionableAudioSource);
        if (d->takeOwnership)
            delete d->src;
    }

    void TalcsPositionableAudioSource::prepareToPlay(int samplesPerBlockExpected, double sampleRate) {
        Q_D(TalcsPositionableAudioSource);
        d->src->open(samplesPerBlockExpected, sampleRate);
    }

    static inline bool inRange(qint64 x, qint64 l, qint64 r) {
        return x >= l && x < r;
    }

    void TalcsPositionableAudioSource::getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) {
        Q_D(TalcsPositionableAudioSource);
        d->wrapper.reset(bufferToFill.buffer->getArrayOfWritePointers(), bufferToFill.buffer->getNumChannels(), bufferToFill.numSamples, bufferToFill.startSample);
        if (d->isLooping) {
            qint64 srcLength = d->src->length();
            qint64 curBufPos = 0;
            qint64 lengthToRead = bufferToFill.numSamples;
            qint64 srcPos = d->src->nextReadPosition();
            while (curBufPos + srcLength - srcPos < bufferToFill.numSamples &&
                   inRange(srcLength, srcPos, srcPos + lengthToRead)) {
                d->src->read({&d->wrapper, curBufPos, srcLength - srcPos});
                curBufPos += srcLength - srcPos;
                lengthToRead -= srcLength - srcPos;
                d->src->setNextReadPosition(0);
                srcPos = 0;
            }
            d->src->read({&d->wrapper, curBufPos, lengthToRead});
        } else {
            d->src->read(&d->wrapper);
        }
    }

    void TalcsPositionableAudioSource::releaseResources() {
        Q_D(TalcsPositionableAudioSource);
        d->src->close();
    }

    juce::int64 TalcsPositionableAudioSource::getTotalLength() const {
        Q_D(const TalcsPositionableAudioSource);
        return d->src->length();
    }

    juce::int64 TalcsPositionableAudioSource::getNextReadPosition() const {
        Q_D(const TalcsPositionableAudioSource);
        return d->src->nextReadPosition();
    }

    void TalcsPositionableAudioSource::setNextReadPosition(juce::int64 newPosition) {
        Q_D(TalcsPositionableAudioSource);
        d->src->setNextReadPosition(newPosition);
    }

    bool TalcsPositionableAudioSource::isLooping() const {
        Q_D(const TalcsPositionableAudioSource);
        return d->isLooping;
    }

    void TalcsPositionableAudioSource::setLooping(bool shouldLoop) {
        Q_D(TalcsPositionableAudioSource);
        d->isLooping = shouldLoop;
    }

    TalcsPositionableAudioSource::TalcsPositionableAudioSource(TalcsPositionableAudioSourcePrivate &d) : d_ptr(&d) {
        d.q_ptr = this;
    }
}
