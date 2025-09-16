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

#include "DspxNoteContext.h"
#include "DspxNoteContext_p.h"

#include <cmath>

#include <limits>

#include <TalcsCore/AudioBuffer.h>

#include <TalcsDspx/private/DspxSingingClipContext_p.h>
#include <TalcsDspx/DspxPseudoSingerContext.h>
#include <TalcsDspx/DspxTrackContext.h>
#include <TalcsDspx/DspxProjectContext.h>

namespace talcs {

    static AudioBuffer dummyBuffer;

    double getFrequencyFromCent(int cent) {
        return 440.0 * std::pow(2.0, (cent / 100.0 - 69.0) / 12.0);
    }

    bool DspxNoteContextSynthesizer::open(qint64 bufferSize, double sampleRate) {
        d->noteSynthesizer->open(bufferSize, sampleRate);
        return AudioSource::open(bufferSize, sampleRate);
    }

    void DspxNoteContextSynthesizer::close() {
        d->noteSynthesizer->close();
        AudioSource::close();
    }

    qint64 DspxNoteContextSynthesizer::length() const {
        return std::numeric_limits<qint64>::max();
    }

    void DspxNoteContextSynthesizer::setNextReadPosition(qint64 pos) {
        if (pos == PositionableAudioSource::nextReadPosition())
            return;
        QMutexLocker locker(&mutex);
        PositionableAudioSource::setNextReadPosition(0);
        d->noteSynthesizer->flush(true);
        d->noteSynthesizer->read({&dummyBuffer, 0, pos, -1});
        PositionableAudioSource::setNextReadPosition(pos);

    }

    void DspxNoteContextSynthesizer::detectInterval(qint64 intervalLength) {
        auto convertTime = d->singingClipContext->pseudoSingerContext()->trackContext()->projectContext()->timeConverter();
        messages.clear();
        auto pos = PositionableAudioSource::nextReadPosition();
        auto length = d->clipView.length() - d->noteSynthesizer->releaseTime();
        auto freq = getFrequencyFromCent(d->keyCent);
        if (pos == 0) {
            messages.append({0, {freq, NoteSynthesizerDetectorMessage::NoteOn}});
        }
        // TODO pitch and energy
        if (pos <= length && pos + intervalLength > length) {
            messages.append({length - pos, {freq, NoteSynthesizerDetectorMessage::NoteOff}});
        }
        messageIterator = messages.cbegin();
    }

    NoteSynthesizerDetectorMessage DspxNoteContextSynthesizer::nextMessage() {
        if (messageIterator != messages.cend()) {
            return *(messageIterator++);
        } else {
            return NoteSynthesizerDetectorMessage::Null;
        }
    }

    void DspxNoteContextSynthesizer::update() {
        QMutexLocker locker(&mutex);
        auto pos = PositionableAudioSource::nextReadPosition();
        PositionableAudioSource::setNextReadPosition(0);
        d->noteSynthesizer->flush(true);
        if (d->isBeingReadByClip()) {
            d->noteSynthesizer->read({&dummyBuffer, 0, pos, -1});
            PositionableAudioSource::setNextReadPosition(pos);
        }
    }

    qint64 DspxNoteContextSynthesizer::processReading(const AudioSourceReadData &readData) {
        QMutexLocker locker(&mutex);
        d->noteSynthesizer->read(readData);
        PositionableAudioSource::setNextReadPosition(PositionableAudioSource::nextReadPosition() +
                                                     readData.length);
        return readData.length;
    }

    bool DspxNoteContextPrivate::isBeingReadByClip() const {
        auto pos = singingClipContext->d_func()->noteClipSeries->nextReadPosition();
        return pos >= clipView.position() && pos < clipView.position() + clipView.length();
    }

    DspxNoteContext::DspxNoteContext(DspxSingingClipContext *singingClipContext) : QObject(singingClipContext), d_ptr(new DspxNoteContextPrivate) {
        Q_D(DspxNoteContext);
        d->q_ptr = this;
        d->synthesizer = std::make_unique<DspxNoteContextSynthesizer>();
        d->noteSynthesizer = std::make_unique<NoteSynthesizer>();
        d->synthesizer->d = d;
        d->noteSynthesizer->setDetector(d->synthesizer.get());
        d->singingClipContext = singingClipContext;
    }

    DspxNoteContext::~DspxNoteContext() {

    }

    DspxSingingClipContext *DspxNoteContext::singingClipContext() const {
        Q_D(const DspxNoteContext);
        return d->singingClipContext;
    }

    void DspxNoteContext::setPos(int tick) {
        Q_D(DspxNoteContext);
        if (d->posTick != tick) {
            d->posTick = tick;
            updatePosition();
        }
    }

    int DspxNoteContext::pos() const {
        Q_D(const DspxNoteContext);
        return d->posTick;
    }

    void DspxNoteContext::setLength(int tick) {
        Q_D(DspxNoteContext);
        if (d->lengthTick != tick) {
            d->lengthTick = tick;
            updatePosition();
        }
    }

    int DspxNoteContext::length() const {
        Q_D(const DspxNoteContext);
        return d->lengthTick;
    }

    void DspxNoteContext::updatePosition() {
        Q_D(DspxNoteContext);
        auto clipSeries = d->singingClipContext->d_func()->noteClipSeries.get();
        auto convertTime = d->singingClipContext->pseudoSingerContext()->trackContext()->projectContext()->timeConverter();
        clipSeries->setClipStartPos(d->clipView, 0);
        auto clipPositionSample = convertTime(d->singingClipContext->start());
        auto firstSample = convertTime(d->singingClipContext->start() + d->posTick) - clipPositionSample;
        auto lastSample = convertTime(d->singingClipContext->start() + d->posTick + d->lengthTick) + d->noteSynthesizer->releaseTime() - clipPositionSample;
        clipSeries->setClipRange(d->clipView, firstSample, qMax(qint64(1), lastSample - firstSample));
        d->synthesizer->update();
    }

    void DspxNoteContext::setKeyCent(int cent) {
        Q_D(DspxNoteContext);
        if (d->keyCent != cent) {
            d->keyCent = cent;
            d->synthesizer->update();
        }
    }

    int DspxNoteContext::keyCent() const {
        Q_D(const DspxNoteContext);
        return d->keyCent;
    }

    void DspxNoteContext::addPitchAnchor(int pos, const QVariant &anchorData) {
        Q_D(DspxNoteContext);
        d->pitchAnchors.insert(pos, anchorData);
        d->synthesizer->update();
    }

    void DspxNoteContext::removePitchAnchor(int pos) {
        Q_D(DspxNoteContext);
        d->pitchAnchors.remove(pos);
        d->synthesizer->update();
    }

    void DspxNoteContext::clearPitchAnchor() {
        Q_D(DspxNoteContext);
        d->pitchAnchors.clear();
        d->synthesizer->update();
    }

    void DspxNoteContext::addEnergyAnchor(int pos, const QVariant &anchorData) {
        Q_D(DspxNoteContext);
        d->pitchAnchors.insert(pos, anchorData);
        d->synthesizer->update();
    }

    void DspxNoteContext::removeEnergyAnchor(int pos) {
        Q_D(DspxNoteContext);
        d->pitchAnchors.remove(pos);
        d->synthesizer->update();
    }

    void DspxNoteContext::clearEnergyAnchor() {
        Q_D(DspxNoteContext);
        d->pitchAnchors.clear();
        d->synthesizer->update();
    }
}
