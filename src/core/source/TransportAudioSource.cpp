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

#include "TransportAudioSource.h"
#include "TransportAudioSource_p.h"

namespace talcs {

    /**
     * @class TransportAudioSource
     * @brief The object takes a PositionableAudioSource object and supports playing and pausing.
     *
     * This is one of the @ref doc/object_binding.md "object-binding" classes.
     *
     * @see PositionableAudioSource
     */

    /**
     * Default constructor
     */
    TransportAudioSource::TransportAudioSource(QObject *parent)
        : TransportAudioSource(*new TransportAudioSourcePrivate, parent) {
    }

    /**
     * Constructor. Initialize the object with an input source.
     */
    TransportAudioSource::TransportAudioSource(PositionableAudioSource *src, bool takeOwnership, QObject *parent)
        : TransportAudioSource(parent) {
        setSource(src, takeOwnership);
    }

    /**
     * Destructor.
     *
     * If the object is not close, it will be closed now.
     */
    TransportAudioSource::~TransportAudioSource() {
        TransportAudioSource::close();
    }

    TransportAudioSource::TransportAudioSource(TransportAudioSourcePrivate &d, QObject *parent)
        : QObject(parent), AudioSource(d) {
    }

    static inline void safeRead(IAudioSampleContainer *dest, qint64 destPos, qint64 length,
                                PositionableAudioSource *src) {
        src->read({dest, destPos, qBound(0ll, src->length() - src->nextReadPosition(), length)});
    }

    static inline bool inRange(qint64 x, qint64 l, qint64 r) {
        return x >= l && x < r;
    }

    qint64 TransportAudioSource::processReading(const AudioSourceReadData &readData) {
        Q_D(TransportAudioSource);
        if (d->position == d->loopingStart && d->position == d->loopingEnd)
            return 0;
        QMutexLocker locker(&d->mutex);
        int channelCount = readData.buffer->channelCount();
        for (int i = 0; i < channelCount; i++) {
            readData.buffer->clear(i, readData.startPos, readData.length);
        }
        if (d->playbackStatus == Paused)
            return readData.length;
        if (d->playbackStatus == AboutToPlay) {
            d->playbackStatus = Playing;
            emit playbackStatusChanged(Playing);
        }
        if (d->bufferingCounter)
            return readData.length;
        if (d->src) {
            qint64 curBufPos = readData.startPos;
            qint64 lengthToRead = readData.length;
            qint64 srcPos = d->position;
            d->src->setNextReadPosition(srcPos);
            while (curBufPos + d->loopingEnd - srcPos < readData.startPos + readData.length &&
                   inRange(d->loopingEnd, srcPos, srcPos + lengthToRead)) {
                safeRead(readData.buffer, curBufPos, d->loopingEnd - srcPos, d->src);
                curBufPos += d->loopingEnd - srcPos;
                lengthToRead -= d->loopingEnd - srcPos;
                d->src->setNextReadPosition(d->loopingStart);
                d->position = d->loopingStart;
                d->_q_positionAboutToChange(d->loopingStart);
                srcPos = d->loopingStart;
            }
            safeRead(readData.buffer, curBufPos, lengthToRead, d->src);
        }
        d->position += readData.length;
        if (readData.length != 0)
            d->_q_positionAboutToChange(d->position);
        if (d->playbackStatus == AboutToPause) {
            d->playbackStatus = Paused;
            emit playbackStatusChanged(Paused);
        }
        return readData.length;
    }

    /**
     * @copydoc AudioSource::open()
     *
     * This function also opens the input source.
     *
     * The object is paused initially.
     */
    bool TransportAudioSource::open(qint64 bufferSize, double sampleRate) {
        Q_D(TransportAudioSource);
        QMutexLocker locker(&d->mutex);
        AudioSource::close();
        d->playbackStatus = Paused;
        if (d->src && !d->src->open(bufferSize, sampleRate)) {
            return false;
        }
        return AudioSource::open(bufferSize, sampleRate);
    }

    /**
     * @copydoc AudioSource::close()
     *
     * This function also closes the input source.
     */
    void TransportAudioSource::close() {
        Q_D(TransportAudioSource);
        if (d->src)
            d->src->close();
        d->playbackStatus = Paused;
        AudioSource::close();
    }

    /**
     * Dynamically reset the input source.
     * @see TransportAudioSource(PositionableAudioSource *, bool, QObject *)
     */
    void TransportAudioSource::setSource(PositionableAudioSource *src, bool takeOwnership) {
        Q_D(TransportAudioSource);
        QMutexLocker locker(&d->mutex);
        d->src.reset(src, takeOwnership);
        if (src) {
            if (isOpen()) {
                src->setNextReadPosition(d->position);
                src->open(bufferSize(), sampleRate());
            }
        }
    }

    /**
     * Gets the input source.
     */
    PositionableAudioSource *TransportAudioSource::source() const {
        Q_D(const TransportAudioSource);
        return d->src;
    }

    /**
     * Starts playing.
     */
    void TransportAudioSource::play() {
        Q_D(TransportAudioSource);
        if (d->playbackStatus != Playing) {
            d->playbackStatus = AboutToPlay;
            emit playbackStatusChanged(AboutToPlay);
        }
    }

    /**
     * @enum TransportAudioSource::PlaybackStatus
     * The playback status of TransportAudioSource
     *
     * @var TransportAudioSource::Paused
     * Paused
     *
     * @var TransportAudioSource::AboutToPlay
     * About to play
     *
     * @var TransportAudioSource::Playing
     * Playing
     *
     * @var TransportAudioSource::AboutToPause
     * About to pause
     */

    /**
     * Gets the playback status.
     */
    TransportAudioSource::PlaybackStatus TransportAudioSource::playbackStatus() const {
        Q_D(const TransportAudioSource);
        return d->playbackStatus;
    }

    /**
     * Gets whether the object is playing.
     */
    bool TransportAudioSource::isPlaying() const {
        Q_D(const TransportAudioSource);
        return d->playbackStatus == Playing;
    }

    /**
     * Pauses playing.
     */
    void TransportAudioSource::pause() {
        Q_D(TransportAudioSource);
        if (d->playbackStatus != Paused) {
            d->playbackStatus = AboutToPause;
            emit playbackStatusChanged(AboutToPause);
        }
    }

    /**
     * Acquires one buffering counter.
     */
    void TransportAudioSource::acquireBuffering() {
        Q_D(TransportAudioSource);
        emit bufferingCounterChanged(++d->bufferingCounter);
    }

    /**
     * Releases one buffering counter.
     */
    void TransportAudioSource::releaseBuffering() {
        Q_D(TransportAudioSource);
        assert(d->bufferingCounter > 0);
        emit bufferingCounterChanged(--d->bufferingCounter);
    }

    /**
     * Gets the value of the buffering counter.
     */
    int TransportAudioSource::bufferingCounter() const {
        Q_D(const TransportAudioSource);
        return d->bufferingCounter;
    }

    /**
     * Gets the position of playback.
     */
    qint64 TransportAudioSource::position() const {
        Q_D(const TransportAudioSource);
        return d->position;
    }

    /**
     * Sets the position of playback.
     */
    void TransportAudioSource::setPosition(qint64 position) {
        Q_D(TransportAudioSource);
        QMutexLocker locker(&d->mutex);
        if (position == d->position)
            return;
        d->_q_positionAboutToChange(position);
        d->position = position;
        if (d->src)
            d->src->setNextReadPosition(d->position);
    }

    /**
     * Gets the length of the input source.
     */
    qint64 TransportAudioSource::length() const {
        Q_D(const TransportAudioSource);
        if (d->src)
            return d->src->length();
        else
            return 0;
    }

    /**
     * Gets the range of looping playback.
     * @return The first value is the left-close start of range, the second value is the right-open end of range. If not
     * looping playing, both values are -1.
     */
    QPair<qint64, qint64> TransportAudioSource::loopingRange() const {
        Q_D(const TransportAudioSource);
        return {d->loopingStart, d->loopingEnd};
    }

    /**
     * Sets the range of looping playback.
     *
     * Note that the end of range should be greater than the start of range, otherwise the behavior will be quite wierd.
     *
     * If not to looping play, set both values to -1.
     * @param l the left-close start of range
     * @param r the right-open end of range
     */
    void TransportAudioSource::setLoopingRange(qint64 l, qint64 r) {
        Q_D(TransportAudioSource);
        QMutexLocker locker(&d->mutex);
        d->loopingStart = l;
        d->loopingEnd = r;
    }

    void TransportAudioSourcePrivate::_q_positionAboutToChange(qint64 pos) {
        Q_Q(TransportAudioSource);
        emit q->positionAboutToChange(pos);
    }

    TransportAudioSourceStateSaver::TransportAudioSourceStateSaver(TransportAudioSource *src): d(new TransportAudioSourceStateSaverPrivate{src, src ? src->position() : 0, src ? src->loopingRange() : QPair<qint64, qint64>()}) {
    }

    TransportAudioSourceStateSaver::~TransportAudioSourceStateSaver() {
        if (d->src) {
            d->src->setPosition(d->position);
            d->src->setLoopingRange(d->loopingRange.first, d->loopingRange.second);
        }
    }
    
}
