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

#include "DspxSingingClipContext.h"
#include "DspxSingingClipContext_p.h"

#include <TalcsCore/PositionableMixerAudioSource.h>

#include <TalcsDspx/DspxProjectContext.h>
#include <TalcsDspx/DspxTrackContext.h>
#include <TalcsDspx/private/DspxPseudoSingerContext_p.h>
#include <TalcsDspx/private/DspxNoteContext_p.h>

namespace talcs {

    DspxSingingClipContext::DspxSingingClipContext(DspxPseudoSingerContext *pseudoSingerContext) : QObject(pseudoSingerContext), d_ptr(new DspxSingingClipContextPrivate) {
        Q_D(DspxSingingClipContext);
        d->q_ptr = this;
        d->controlMixer = std::make_unique<PositionableMixerAudioSource>();
        d->noteClipSeries = std::make_unique<AudioSourceClipSeries>();

        d->controlMixer->addSource(d->noteClipSeries.get());

        d->pseudoSingerContext = pseudoSingerContext;
    }

    DspxSingingClipContext::~DspxSingingClipContext() {

    }

    DspxPseudoSingerContext *DspxSingingClipContext::pseudoSingerContext() const {
       Q_D(const DspxSingingClipContext);
       return d->pseudoSingerContext;
    }

    PositionableMixerAudioSource *DspxSingingClipContext::controlMixer() const {
        Q_D(const DspxSingingClipContext);
        return d->controlMixer.get();
    }

    void DspxSingingClipContext::setStart(int tick) {
        Q_D(DspxSingingClipContext);
        if (d->startTick != tick) {
            d->startTick = tick;
            updatePosition();
        }
    }

    int DspxSingingClipContext::start() const {
        Q_D(const DspxSingingClipContext);
        return d->startTick;
    }

    void DspxSingingClipContext::setClipStart(int tick) {
        Q_D(DspxSingingClipContext);
        if (d->clipStartTick != tick) {
            d->clipStartTick = tick;
            updatePosition();
        }
    }

    int DspxSingingClipContext::clipStart() const {
        Q_D(const DspxSingingClipContext);
        return d->clipStartTick;
    }

    void DspxSingingClipContext::setClipLen(int tick) {
        Q_D(DspxSingingClipContext);
        if (d->clipLenTick != tick) {
            d->clipLenTick = tick;
            updatePosition();
        }
    }

    int DspxSingingClipContext::clipLen() const {
        Q_D(const DspxSingingClipContext);
        return d->clipLenTick;
    }

    void DspxSingingClipContext::updatePosition() {
        Q_D(DspxSingingClipContext);
        auto clipSeries = d->pseudoSingerContext->d_func()->clipSeries.get();
        auto convertTime = d->pseudoSingerContext->trackContext()->projectContext()->timeConverter();
        auto startSample = convertTime(d->clipStartTick);
        clipSeries->setClipStartPos(d->clipView, startSample);
        auto firstSample = convertTime(d->startTick + d->clipStartTick);
        auto lastSample = convertTime(d->startTick + d->clipStartTick + d->clipLenTick);
        clipSeries->setClipRange(d->clipView, firstSample, qMax(qint64(1), lastSample - firstSample));
    }

    void DspxSingingClipContext::setData(const QVariant &data) {
        Q_D(DspxSingingClipContext);
        d->data = data;
    }

    QVariant DspxSingingClipContext::data() const {
        Q_D(const DspxSingingClipContext);
        return d->data;
    }

    DspxNoteContext *DspxSingingClipContext::addNote(int id) {
        Q_D(DspxSingingClipContext);
        auto note = new DspxNoteContext(this);
        auto noteClipView = d->noteClipSeries->insertClip(note->d_func()->synthesizer.get(), 0, 0, 1);
        d->notes.insert(id, note);
        note->d_func()->clipView = noteClipView;
        note->d_func()->noteSynthesizer->setConfig(d->pseudoSingerContext->config());
        return note;
    }

    void DspxSingingClipContext::removeNote(int id) {
        Q_D(DspxSingingClipContext);
        Q_ASSERT(d->notes.contains(id));
        auto note = d->notes.take(id);
        d->noteClipSeries->removeClip(note->d_func()->clipView);
        delete note;
    }

    QList<DspxNoteContext *> DspxSingingClipContext::notes() const {
        Q_D(const DspxSingingClipContext);
        return d->notes.values();
    }
}