/******************************************************************************
 * Copyright (c) 2023 CrSjimo                                                 *
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

#include "AudioSourceClipSeries.h"
#include "AudioSourceClipSeries_p.h"

#include <QDebug>

namespace talcs {

    AudioSourceClipSeriesPrivate::AudioSourceClipSeriesPrivate()
        : IClipSeriesPrivate(new IClipSeriesRangeResetter(this)), AudioSourceClipSeriesBase(this) {
    }

    /**
     * @class AudioSourceClipSeries
     * @brief An AudioClipsSeriesBase object that uses PositionableAudioSource
     *
     * This is one of the @ref doc/object_binding.md "object-binding" classes.
     */

    /**
     * Default constructor.
     */
    AudioSourceClipSeries::AudioSourceClipSeries()
        : AudioSourceClipSeries(*new AudioSourceClipSeriesPrivate) {
    }
    AudioSourceClipSeries::AudioSourceClipSeries(AudioSourceClipSeriesPrivate &d)
        : PositionableAudioSource(d) {
    }

    /**
     * Destructor.
     *
     * If the object is not close, it will be close now.
     */
    AudioSourceClipSeries::~AudioSourceClipSeries() {
        AudioSourceClipSeries::close();
    }
    qint64 AudioSourceClipSeries::read(const AudioSourceReadData &readData) {
        Q_D(AudioSourceClipSeries);
        AudioSourceClipSeriesPrivate::ClipInterval readDataInterval(0, d->position, readData.length);
        for (int ch = 0; ch < readData.buffer->channelCount(); ch++) {
            readData.buffer->clear(ch, readData.startPos, readData.length);
        }
        qAsConst(d->clips).overlap_find_all(
            readDataInterval, [=](const decltype(d->clips)::const_iterator &it) {
                auto clip = it->interval();
                auto [clipReadPosition, clipReadData] =
                    d->calculateClipReadData(clip, d->position, readData);
                auto clipSrc = reinterpret_cast<PositionableAudioSource *>(clip.content());
                clipSrc->setNextReadPosition(clipReadPosition);
                clipSrc->read(clipReadData);
                return true;
            });
        d->position += readData.length;
        return readData.length;
    }
    qint64 AudioSourceClipSeries::length() const {
        return std::numeric_limits<qint64>::max();
    }
    qint64 AudioSourceClipSeries::nextReadPosition() const {
        Q_D(const AudioSourceClipSeries);
        return d->position;
    }
    void AudioSourceClipSeries::setNextReadPosition(qint64 pos) {
        Q_D(AudioSourceClipSeries);
        QMutexLocker locker(&d->mutex);
        d->position = pos;
    }

    /**
     * @copydoc PositionableAudioSource::open()
     *
     * This function also opens all sources in clips.
     */
    bool AudioSourceClipSeries::open(qint64 bufferSize, double sampleRate) {
        Q_D(AudioSourceClipSeries);
        QMutexLocker locker(&d->mutex);
        if (d->openAllClips(bufferSize, sampleRate))
            return AudioStreamBase::open(bufferSize, sampleRate);
        return false;
    }

    /**
     * @copydoc PositionableAudioSource::close()
     *
     * This function also closes all sources in clips.
     */
    void AudioSourceClipSeries::close() {
        Q_D(AudioSourceClipSeries);
        QMutexLocker locker(&d->mutex);
        d->closeAllClips();
        AudioStreamBase::close();
    }

    AudioSourceClipSeries::ClipView
    AudioSourceClipSeries::insertClip(PositionableAudioSource *content, qint64 position, qint64 startPos,
                                      qint64 length) {
        Q_D(AudioSourceClipSeries);
        QMutexLocker locker(&d->mutex);
        if (!d->preInsertClip(content))
            return ClipView(d->nullClipViewImpl());
        return ClipView(d->insertClip(reinterpret_cast<qintptr>(content), position, startPos, length));
    }

    AudioSourceClipSeries::ClipView
    AudioSourceClipSeries::findClip(PositionableAudioSource *content) const {
        Q_D(const AudioSourceClipSeries);
        return ClipView(d->findClipByContent(reinterpret_cast<qintptr>(content)));
    }

    AudioSourceClipSeries::ClipView AudioSourceClipSeries::findClip(qint64 position) const {
        Q_D(const AudioSourceClipSeries);
        return ClipView(d->findClipByPosition(position));
    }

    void AudioSourceClipSeries::removeClip(const AudioSourceClipSeries::ClipView &clip) {
        Q_D(AudioSourceClipSeries);
        QMutexLocker locker(&d->mutex);
        d->removeClip(clip);
    }

    void AudioSourceClipSeries::removeAllClips() {
        Q_D(AudioSourceClipSeries);
        QMutexLocker locker(&d->mutex);
        d->removeAllClips();
    }

    QList<AudioSourceClipSeries::ClipView> AudioSourceClipSeries::clips() const {
        Q_D(const AudioSourceClipSeries);
        QList<ClipView> list;
        for (const auto &impl : d->clipViewImplList())
            list.append(ClipView(impl));
        return list;
    }

    qint64 AudioSourceClipSeries::effectiveLength() const {
        Q_D(const AudioSourceClipSeries);
        return d->effectiveLength();
    }

}
