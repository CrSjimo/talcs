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

    AudioSourceClipSeriesPrivate::AudioSourceClipSeriesPrivate() : AudioSourceClipSeriesBase(this) {
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
    qint64 AudioSourceClipSeries::processReading(const AudioSourceReadData &readData) {
        Q_D(AudioSourceClipSeries);
        AudioSourceClipSeriesPrivate::ClipInterval readDataInterval(nullptr, d->position, readData.length);
        for (int ch = 0; ch < readData.buffer->channelCount(); ch++) {
            readData.buffer->clear(ch, readData.startPos, readData.length);
        }
        qAsConst(d->clips).overlap_find_all(
            readDataInterval, [=](const decltype(d->clips)::const_iterator &it) {
                auto clip = it->interval();
                auto [clipReadPosition, clipReadData] =
                    d->calculateClipReadData(clip, d->position, readData);
                auto clipSrc = static_cast<PositionableAudioSource *>(clip.content());
                clipSrc->setNextReadPosition(clipReadPosition);
                clipSrc->read(clipReadData);
                for (int ch = 0; ch < readData.buffer->channelCount(); ch++)
                    readData.buffer->addSampleRange(ch, readData.startPos, readData.length, d->buf, ch, 0);
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
        AudioSource::close();
        if (d->openAllClips(bufferSize, sampleRate))
            return AudioSource::open(bufferSize, sampleRate);
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
        AudioSource::close();
    }

    AudioSourceClipSeries::ClipView
    AudioSourceClipSeries::insertClip(PositionableAudioSource *content, qint64 position, qint64 startPos,
                                      qint64 length) {
        Q_D(AudioSourceClipSeries);
        QMutexLocker locker(&d->mutex);
        if (!d->preInsertClip(content))
            return {};
        return d->insertClip(content, position, startPos, length);
    }

    void AudioSourceClipSeries::setClipStartPos(const AudioSourceClipSeries::ClipView &clip,
                                                qint64 startPos) {
        Q_D(AudioSourceClipSeries);
        d->setClipStartPos(clip, startPos);
    }

    bool AudioSourceClipSeries::setClipRange(const AudioSourceClipSeries::ClipView &clip, qint64 position,
                                             qint64 length) {
        Q_D(AudioSourceClipSeries);
        return d->setClipRange(clip, position, length);
    }

    bool AudioSourceClipSeries::setClipContent(const AudioSourceClipSeries::ClipView &clip,
                                          PositionableAudioSource *content) {
        Q_D(AudioSourceClipSeries);
        QMutexLocker locker(&d->mutex);
        if (!d->preInsertClip(content))
            return false;
        return d->setClipContent(clip, content);
    }

    AudioSourceClipSeries::ClipView
    AudioSourceClipSeries::findClip(PositionableAudioSource *content) const {
        Q_D(const AudioSourceClipSeries);
        return d->findClipByContent(content);
    }

    QList<AudioSourceClipSeries::ClipView> AudioSourceClipSeries::findClip(qint64 position) const {
        Q_D(const AudioSourceClipSeries);
        QList<ClipView> list;
        d->findClipByPosition(position, [&](const ClipViewPrivate::ClipViewImpl &clipView) {
            list.append(clipView);
            return true;
        });
        return list;
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
