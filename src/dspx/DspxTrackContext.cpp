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

#include "DspxTrackContext.h"
#include "DspxTrackContext_p.h"

#include <TalcsCore/PositionableMixerAudioSource.h>
#include <TalcsCore/AudioSourceClipSeries.h>

#include <TalcsDspx/DspxProjectContext.h>
#include <TalcsDspx/DspxAudioClipContext.h>
#include <TalcsDspx/private/DspxAudioClipContext_p.h>

namespace talcs {
    DspxTrackContext::DspxTrackContext(DspxProjectContext *projectContext) : QObject(projectContext), d_ptr(new DspxTrackContextPrivate) {
        Q_D(DspxTrackContext);
        d->q_ptr = this;

        d->controlMixer = std::make_unique<PositionableMixerAudioSource>();
        d->trackMixer = std::make_unique<PositionableMixerAudioSource>();
        d->clipSeries = std::make_unique<AudioSourceClipSeries>();

        d->trackMixer->addSource(d->clipSeries.get());
        d->controlMixer->addSource(d->trackMixer.get());

        d->projectContext = projectContext;
    }

    DspxTrackContext::~DspxTrackContext() {

    }

    PositionableMixerAudioSource *DspxTrackContext::controlMixer() const {
        Q_D(const DspxTrackContext);
        return d->controlMixer.get();
    }

    PositionableMixerAudioSource *DspxTrackContext::trackMixer() const {
        Q_D(const DspxTrackContext);
        return d->trackMixer.get();
    }

    AudioSourceClipSeries *DspxTrackContext::clipSeries() const {
        Q_D(const DspxTrackContext);
        return d->clipSeries.get();
    }

    DspxProjectContext *DspxTrackContext::projectContext() const {
        Q_D(const DspxTrackContext);
        return d->projectContext;
    }

    void DspxTrackContext::setData(const QVariant &data) {
        Q_D(DspxTrackContext);
        d->data = data;
    }

    QVariant DspxTrackContext::data() const {
        Q_D(const DspxTrackContext);
        return d->data;
    }

    DspxAudioClipContext *DspxTrackContext::addAudioClip(int id) {
        Q_D(DspxTrackContext);
        auto clip = new DspxAudioClipContext(this);
        auto clipView = d->clipSeries->insertClip(clip->controlMixer(), 0, 0, 1);
        d->clips.insert(id, clip);
        clip->d_func()->clipView = clipView;
        return clip;
    }

    void DspxTrackContext::removeAudioClip(int id) {
        Q_D(DspxTrackContext);
        Q_ASSERT(d->clips.contains(id));
        std::unique_ptr<DspxAudioClipContext> clip(d->clips.take(id));
        d->clipSeries->removeClip(clip->d_func()->clipView);
    }

    QList<DspxAudioClipContext *> DspxTrackContext::clips() const {
        Q_D(const DspxTrackContext);
        return d->clips.values();
    }
} // talcs