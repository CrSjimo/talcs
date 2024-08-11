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

#include "DspxPseudoSingerContext.h"
#include "DspxPseudoSingerContext_p.h"

#include <TalcsCore/PositionableMixerAudioSource.h>
#include <TalcsCore/AudioSourceClipSeries.h>

#include <TalcsDspx/DspxTrackContext.h>
#include <TalcsDspx/private/DspxSingingClipContext_p.h>
#include <TalcsDspx/DspxNoteContext.h>

namespace talcs {
    DspxPseudoSingerContext::DspxPseudoSingerContext(DspxTrackContext *trackContext) : QObject(trackContext), d_ptr(new DspxPseudoSingerContextPrivate) {
        Q_D(DspxPseudoSingerContext);
        d->q_ptr = this;
        d->clipSeries = std::make_unique<AudioSourceClipSeries>();
        trackContext->trackMixer()->addSource(d->clipSeries.get());
        d->trackContext = trackContext;
    }

    DspxPseudoSingerContext::~DspxPseudoSingerContext() {
        Q_D(DspxPseudoSingerContext);
        d->trackContext->trackMixer()->removeSource(d->clipSeries.get());
    }

    DspxTrackContext *DspxPseudoSingerContext::trackContext() const {
        Q_D(const DspxPseudoSingerContext);
        return d->trackContext;
    }

    DspxSingingClipContext *DspxPseudoSingerContext::addSingingClip(int id) {
        Q_D(DspxPseudoSingerContext);
        auto clip = new DspxSingingClipContext(this);
        auto clipView = d->clipSeries->insertClip(clip->controlMixer(), 0, 0, 1);
        d->clips.insert(id, clip);
        clip->d_func()->clipView = clipView;
        return clip;
    }

    void DspxPseudoSingerContext::removeSingingClip(int id) {
        Q_D(DspxPseudoSingerContext);
        Q_ASSERT(d->clips.contains(id));
        auto clip = d->clips.value(id);
        d->clips.remove(id);
        d->clipSeries->removeClip(clip->d_func()->clipView);
        delete clip;
    }

    QList<DspxSingingClipContext *> DspxPseudoSingerContext::clips() const {
        Q_D(const DspxPseudoSingerContext);
        return d->clips.values();
    }

    void DspxPseudoSingerContext::setConfig(const NoteSynthesizerConfig &config) {

    }

    NoteSynthesizerConfig DspxPseudoSingerContext::config() const {
        Q_D(const DspxPseudoSingerContext);
        return d->config;
    }
}