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

#include "DspxAudioClipContext.h"
#include "DspxAudioClipContext_p.h"

#include <TalcsCore/PositionableMixerAudioSource.h>
#include <TalcsCore/BufferingAudioSource.h>

#include <TalcsFormat/FormatManager.h>
#include <TalcsFormat/FormatEntry.h>
#include <TalcsFormat/AudioFormatInputSource.h>

#include <TalcsDspx/DspxTrackContext.h>
#include <TalcsDspx/DspxProjectContext.h>

namespace talcs {

    DspxAudioClipContext::DspxAudioClipContext(DspxTrackContext *trackContext) : QObject(trackContext), d_ptr(new DspxAudioClipContextPrivate) {
        Q_D(DspxAudioClipContext);
        d->q_ptr = this;

        d->controlMixer = std::make_unique<PositionableMixerAudioSource>();
        d->clipMixer = std::make_unique<PositionableMixerAudioSource>();

        d->controlMixer->addSource(d->clipMixer.get());

        d->trackContext = trackContext;
    }

    DspxAudioClipContext::~DspxAudioClipContext() {

    }

    PositionableMixerAudioSource *DspxAudioClipContext::controlMixer() const {
        Q_D(const DspxAudioClipContext);
        return d->controlMixer.get();
    }

    PositionableMixerAudioSource *DspxAudioClipContext::clipMixer() const {
        Q_D(const DspxAudioClipContext);
        return d->clipMixer.get();
    }

    BufferingAudioSource *DspxAudioClipContext::contentSource() const {
        Q_D(const DspxAudioClipContext);
        return d->contentSource.get();
    }

    DspxTrackContext *DspxAudioClipContext::trackContext() const {
        Q_D(const DspxAudioClipContext);
        return d->trackContext;
    }

    void DspxAudioClipContext::setStart(int tick) {
        Q_D(DspxAudioClipContext);
        if (d->startTick != tick) {
            d->startTick = tick;
            updatePosition();
        }
    }

    int DspxAudioClipContext::start() const {
        Q_D(const DspxAudioClipContext);
        return d->startTick;
    }

    void DspxAudioClipContext::setClipStart(int tick) {
        Q_D(DspxAudioClipContext);
        if (d->clipStartTick != tick) {
            d->clipStartTick = tick;
            updatePosition();
        }
    }

    int DspxAudioClipContext::clipStart() const {
        Q_D(const DspxAudioClipContext);
        return d->clipStartTick;
    }

    void DspxAudioClipContext::setClipLen(int tick) {
        Q_D(DspxAudioClipContext);
        if (d->clipLenTick != tick) {
            d->clipLenTick = tick;
            updatePosition();
        }
    }

    int DspxAudioClipContext::clipLen() const {
        Q_D(const DspxAudioClipContext);
        return d->clipLenTick;
    }

    void DspxAudioClipContextPrivate::handleIO(AbstractAudioFormatIO *io) {
        auto rawSource_ = std::make_unique<AudioFormatInputSource>(io, true);
        if (contentSource)
            clipMixer->removeSource(contentSource.get());
        contentSource.reset(trackContext->projectContext()->makeBufferable(rawSource_.get(), 2));
        rawSource = std::move(rawSource_);
        clipMixer->prependSource(contentSource.get());
    }

    bool DspxAudioClipContext::setPathLoad(const QString &path, const QVariant &data, const QString &entryClassName) {
        Q_D(DspxAudioClipContext);
        auto formatManager = d->trackContext->projectContext()->formatManager();
        if (!formatManager)
            return false;
        auto io = formatManager->getFormatLoad(path, data);
        if (!io)
            return false;
        d->handleIO(io);
        d->path = path;
        return true;
    }

    bool DspxAudioClipContext::setPathOpen(const QString &path, AbstractAudioFormatIO *io) {
        Q_D(DspxAudioClipContext);
        if (!io)
            return false;
        d->handleIO(io);
        d->path = path;
        return true;
    }

    QString DspxAudioClipContext::path() const {
        Q_D(const DspxAudioClipContext);
        return d->path;
    }

    void DspxAudioClipContext::updatePosition() {
        Q_D(DspxAudioClipContext);
        auto clipSeries = d->trackContext->clipSeries();
        auto convertTime = d->trackContext->projectContext()->timeConverter();
        auto startSample = convertTime(d->clipStartTick);
        clipSeries->setClipStartPos(d->clipView, startSample);
        auto firstSample = convertTime(d->startTick + d->clipStartTick);
        auto lastSample = convertTime(d->startTick + d->clipStartTick + d->clipLenTick);
        clipSeries->setClipRange(d->clipView, firstSample, qMax(qint64(1), lastSample - firstSample));
    }

    void DspxAudioClipContext::setData(const QVariant &data) {
        Q_D(DspxAudioClipContext);
        d->data = data;
    }

    QVariant DspxAudioClipContext::data() const {
        Q_D(const DspxAudioClipContext);
        return d->data;
    }

} // talcs
