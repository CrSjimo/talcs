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

#include "DspxProjectContext.h"
#include "DspxProjectContext_p.h"

#include <TalcsDspx/DspxTrackContext.h>

namespace talcs {

    DspxProjectContextBufferingAudioSourceObject::DspxProjectContextBufferingAudioSourceObject(PositionableAudioSource *src, int channelCount, qint64 readAheadSize, QObject *parent)
    : QObject(parent), BufferingAudioSource(src, channelCount, readAheadSize) {
    }

    DspxProjectContextBufferingAudioSourceObject::~DspxProjectContextBufferingAudioSourceObject() = default;

    DspxProjectContext::DspxProjectContext(QObject *parent) : QObject(parent), d_ptr(new DspxProjectContextPrivate) {
        Q_D(DspxProjectContext);
        d->q_ptr = this;

        d->preMixer = std::make_unique<MixerAudioSource>();
        d->transport = std::make_unique<TransportAudioSource>();
        d->postMixer = std::make_unique<PositionableMixerAudioSource>();
        d->masterControlMixer = std::make_unique<PositionableMixerAudioSource>();
        d->masterTrackMixer = std::make_unique<PositionableMixerAudioSource>();

        d->masterControlMixer->addSource(d->masterTrackMixer.get());
        d->postMixer->addSource(d->masterControlMixer.get());
        d->transport->setSource(d->postMixer.get());
        d->preMixer->addSource(d->transport.get());
    }

    DspxProjectContext::~DspxProjectContext() = default;

    MixerAudioSource *DspxProjectContext::preMixer() const {
        Q_D(const DspxProjectContext);
        return d->preMixer.get();
    }

    TransportAudioSource *DspxProjectContext::transport() const {
        Q_D(const DspxProjectContext);
        return d->transport.get();
    }

    PositionableMixerAudioSource *DspxProjectContext::postMixer() const {
        Q_D(const DspxProjectContext);
        return d->postMixer.get();
    }

    PositionableMixerAudioSource *DspxProjectContext::masterControlMixer() const {
        Q_D(const DspxProjectContext);
        return d->masterControlMixer.get();
    }

    PositionableMixerAudioSource *DspxProjectContext::masterTrackMixer() const {
        Q_D(const DspxProjectContext);
        return d->masterTrackMixer.get();
    }

    void DspxProjectContext::setFormatManager(FormatManager *formatManager) {
        Q_D(DspxProjectContext);
        d->formatManager = formatManager;
    }

    FormatManager *DspxProjectContext::formatManager() const {
        Q_D(const DspxProjectContext);
        return d->formatManager;
    }

    void DspxProjectContext::setTimeConverter(const DspxProjectContext::TimeConverter &converter) {
        Q_D(DspxProjectContext);
        d->timeConverter = converter;
    }

    DspxProjectContext::TimeConverter DspxProjectContext::timeConverter() const {
        Q_D(const DspxProjectContext);
        return d->timeConverter;
    }

    void DspxProjectContext::setInterpolator(const DspxProjectContext::Interpolator &interpolator) {
        Q_D(DspxProjectContext);
        d->interpolator = interpolator;
    }

    DspxProjectContext::Interpolator DspxProjectContext::interpolator() const {
        Q_D(const DspxProjectContext);
        return d->interpolator;
    }

    void DspxProjectContext::setBufferingReadAheadSize(qint64 size) {
        Q_D(DspxProjectContext);
        if (size != d->bufferingReadAheadSize) {
            d->bufferingReadAheadSize = size;
            emit d->readAheadSizeChanged(size);
        }
    }

    qint64 DspxProjectContext::bufferingReadAheadSize() const {
        Q_D(const DspxProjectContext);
        return d->bufferingReadAheadSize;
    }

    BufferingAudioSource *DspxProjectContext::makeBufferable(PositionableAudioSource *source, int channelCount) {
        Q_D(DspxProjectContext);
        auto bufSrc = new DspxProjectContextBufferingAudioSourceObject(source, channelCount, d->bufferingReadAheadSize);
        connect(d, &DspxProjectContextPrivate::readAheadSizeChanged, bufSrc, [bufSrc](qint64 size) {
            bufSrc->setReadAheadSize(size);
        });
        return bufSrc;
    }

    DspxTrackContext *DspxProjectContext::addTrack(int index) {
        Q_D(DspxProjectContext);
        Q_ASSERT(index >= 0 && index <= d->tracks.size());
        auto track = new DspxTrackContext(this);
        d->tracks.insert(index, track);
        d->masterTrackMixer->insertSource(d->masterTrackMixer->sourceAt(index), track->controlMixer());
        return track;
    }

    void DspxProjectContext::removeTrack(int index) {
        Q_D(DspxProjectContext);
        Q_ASSERT(index >= 0 && index < d->tracks.size());
        auto track = d->tracks.at(index);
        d->tracks.removeAt(index);
        d->masterTrackMixer->removeSource(track->controlMixer());
        delete track;
    }

    void DspxProjectContext::moveTrack(int index, int count, int dest) {
        Q_D(DspxProjectContext);
        Q_ASSERT(index >= 0 && index < d->tracks.size() && count > 0 && index + count <= d->tracks.size() && dest >= 0 && dest <= d->tracks.size());

        QVector<DspxTrackContext *> tmp;
        tmp.resize(count);
        std::copy(d->tracks.begin() + index, d->tracks.begin() + index + count, tmp.begin());
        int correctDest;
        if (dest > index) {
            correctDest = dest - count;
            auto sz = correctDest - index;
            for (int i = 0; i < sz; ++i) {
                d->tracks[index + i] = d->tracks[index + count + i];
            }
        } else {
            correctDest = dest;
            auto sz = index - dest;
            for (int i = sz - 1; i >= 0; --i) {
                d->tracks[dest + count + i] = d->tracks[dest + i];
            }
        }
        std::copy(tmp.begin(), tmp.end(), d->tracks.begin() + correctDest);

        d->masterTrackMixer->moveSource(d->masterTrackMixer->sourceAt(dest), d->masterTrackMixer->sourceAt(index), d->masterTrackMixer->sourceAt(index + count));
    }

    QList<DspxTrackContext *> DspxProjectContext::tracks() const {
        Q_D(const DspxProjectContext);
        return d->tracks;
    }
}