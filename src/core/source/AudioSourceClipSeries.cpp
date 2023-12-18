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

    AudioSourceClipSeriesPrivate::AudioSourceClipSeriesPrivate(AudioSourceClipSeries *q)
        : AudioSourceClipSeriesImpl(q) {
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
        : AudioSourceClipSeries(*new AudioSourceClipSeriesPrivate(this)) {
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
        AudioSourceClip readDataInterval(d->position, readData.length);
        for (int ch = 0; ch < readData.buffer->channelCount(); ch++) {
            readData.buffer->clear(ch, readData.startPos, readData.length);
        }
        qAsConst(m_clips).overlap_find_all(
            readDataInterval, [=, &readDataInterval](const decltype(m_clips)::const_iterator &it) {
                auto clip = it->interval();
                auto [clipReadPosition, clipReadInterval] =
                    calculateClipReadData(clip, readDataInterval);
                clip.content()->setNextReadPosition(clipReadPosition);
                clip.content()->read({
                    readData.buffer,
                    clipReadInterval.position() + readData.startPos,
                    clipReadInterval.length(),
                    readData.silentFlags,
                });
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
        if (d->open(bufferSize, sampleRate))
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
        d->close();
        AudioStreamBase::close();
    }

    bool AudioSourceClipSeries::addClip(const AudioSourceClip &clip) {
        Q_D(AudioSourceClipSeries);
        QMutexLocker locker(&d->mutex);
        if (AudioClipSeriesBase::addClip(clip))
            return d->addClip(clip);
        return false;
    }
    bool AudioSourceClipSeries::removeClipAt(qint64 pos) {
        Q_D(AudioSourceClipSeries);
        QMutexLocker locker(&d->mutex);
        auto clip = AudioClipSeriesBase::findClipAt(pos);
        if (AudioClipSeriesBase::removeClipAt(pos)) {
            d->removeClip(clip);
            return true;
        }
        return false;
    }
    void AudioSourceClipSeries::clearClips() {
        Q_D(AudioSourceClipSeries);
        QMutexLocker locker(&d->mutex);
        d->clearClips();
        AudioClipSeriesBase::clearClips();
    }
    
}
