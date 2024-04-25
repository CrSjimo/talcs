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

#include "BufferingAudioSource.h"
#include "BufferingAudioSource_p.h"

#include <QDebug>
#include <QThreadPool>

namespace talcs {

    /**
     * @class BufferingAudioSource
     * @brief Buffering when reading from a PositionableAudioSource
     */

    /**
     * Constructor.
     * @overload
     */
    BufferingAudioSource::BufferingAudioSource(PositionableAudioSource *src, int channelCount, qint64 readAheadSize,
                                               bool autoBuffering, QThreadPool *threadPool) : BufferingAudioSource(src, false, channelCount,
                                                                                               readAheadSize, autoBuffering,
                                                                                               threadPool) {

    }

    /**
     * Constructor.
     * @param src the PositionableAudioSource object from which to read
     * @param takeOwnership whether to take the ownership of the source
     * @param channelCount the number of channel to read
     * @param readAheadSize the size of buffering (if less than bufferSize(), then it will read directly)
     * @param autoBuffering automatically starts buffering after the source is opened
     * @param threadPool the thread pool in which runs the buffering task
     */
    BufferingAudioSource::BufferingAudioSource(PositionableAudioSource *src, bool takeOwnership, int channelCount,
                                               qint64 readAheadSize, bool autoBuffering, QThreadPool *threadPool) : BufferingAudioSource(*new BufferingAudioSourcePrivate) {
        Q_D(BufferingAudioSource);
        d->src = src;
        d->takeOwnership = takeOwnership;
        d->channelCount = channelCount;
        d->readAheadSize = readAheadSize;
        d->autoBuffering = autoBuffering;
        d->threadPool = threadPool ? threadPool : BufferingAudioSource::threadPool();

        d->headPosition = 0;
        d->tailPosition = 0;
    }

    /**
     * Destructor.
     */
    BufferingAudioSource::~BufferingAudioSource() {
        BufferingAudioSource::close();
    }

    qint64 BufferingAudioSource::processReading(const AudioSourceReadData &readData) {
        Q_D(BufferingAudioSource);
        QMutexLocker locker(&d->mutex);
        Q_ASSERT(isOpen());
        if (d->readAheadSize <= bufferSize()) {
            return d->src->read(readData);
        }
        int channelCount = qMin(readData.buffer->channelCount(), d->channelCount);
        {
            QMutexLocker readLocker(&d->bufLock);
            qint64 head = d->headPosition;
            qint64 tail = d->tailPosition;
            if (tail - head >= readData.length) {
                readFromBuffer:
                for (int ch = 0; ch < channelCount; ch++) {
                    readData.buffer->setSampleRange(ch, readData.startPos, readData.length, d->buf, ch, head);
                }
                for (int ch = channelCount; ch < readData.buffer->channelCount(); ch++) {
                    readData.buffer->clear(ch, readData.startPos, readData.length);
                }
            } else {
                readLocker.unlock();
                d->accelerateCurrentBufferingTaskAndWait();
                readLocker.relock();
                head = d->headPosition;
                tail = d->tailPosition;
                Q_ASSERT(tail - head >= readData.length);
                goto readFromBuffer;
            }
            d->headPosition += readData.length;
        }
        if (!d->currentBufferingTask)
            d->commitBufferingTask(false);
        d->position += readData.length;
        return readData.length;
    }

    qint64 BufferingAudioSource::length() const {
        Q_D(const BufferingAudioSource);
        return d->src->length();
    }

    void BufferingAudioSource::setNextReadPosition(qint64 pos) {
        Q_D(BufferingAudioSource);
        QMutexLocker locker(&d->mutex);
        if (pos == nextReadPosition())
            return;
        if (isOpen() && d->readAheadSize > bufferSize()) {
            flush();
            d->src->setNextReadPosition(pos);
            if (d->autoBuffering)
                d->commitBufferingTask(false);
        } else {
            d->src->setNextReadPosition(pos);
        }
        PositionableAudioSource::setNextReadPosition(pos);
    }

    bool BufferingAudioSource::open(qint64 bufferSize, double sampleRate) {
        Q_D(BufferingAudioSource);
        QMutexLocker locker(&d->mutex);
        flush();
        if (!d->src->open(bufferSize, sampleRate))
            return false;
        d->src->setNextReadPosition(d->position);
        if (d->readAheadSize > bufferSize) {
            d->buf.resize(d->channelCount, d->readAheadSize * 2);
            if (d->autoBuffering)
                d->commitBufferingTask(false);
        }
        return AudioSource::open(bufferSize, sampleRate);
    }

    void BufferingAudioSource::close() {
        Q_D(BufferingAudioSource);
        QMutexLocker locker(&d->mutex);
        flush();
        AudioSource::close();
    }

    void BufferingAudioSource::setReadAheadSize(qint64 size) {
        Q_D(BufferingAudioSource);
        QMutexLocker locker(&d->mutex);
        if (size == d->readAheadSize)
            return;
        if (isOpen() && size > bufferSize()) {
            flush();
            d->readAheadSize = size;
            d->buf.resize(-1, size * 2);
            if (d->autoBuffering)
                d->commitBufferingTask(false);
        } else {
            d->readAheadSize = size;
        }
    }

    /**
     * Gets the buffering size.
     */
    qint64 BufferingAudioSource::readAheadSize() const {
        Q_D(const BufferingAudioSource);
        return d->readAheadSize;
    }

    /**
     * Sets the number of channels.
     */
    void BufferingAudioSource::setChannelCount(int channelCount) {
        Q_D(BufferingAudioSource);
        QMutexLocker locker(&d->mutex);
        if (channelCount == d->channelCount)
            return;
        if (isOpen() && d->readAheadSize > bufferSize()) {
            flush();
            d->channelCount = channelCount;
            d->buf.resize(channelCount);
            if (d->autoBuffering)
                d->commitBufferingTask(false);
        } else {
            d->channelCount = channelCount;
        }
    }

    /**
     * Gets the number of channels.
     */
    int BufferingAudioSource::channelCount() const {
        Q_D(const BufferingAudioSource);
        return d->channelCount;
    }

    /**
     * Sets the source from which to read from.
     */
    void BufferingAudioSource::setSource(PositionableAudioSource *src, bool takeOwnership) {
        Q_D(BufferingAudioSource);
        QMutexLocker locker(&d->mutex);
        if (isOpen() && d->readAheadSize > bufferSize()) {
            flush();
            if (!src->open(bufferSize(), sampleRate())) {
                qWarning() << "BufferingAudioSource: Cannot open source";
                return;
            }
            d->src = src;
            d->takeOwnership = takeOwnership;
            d->src->setNextReadPosition(d->position);
            if (d->autoBuffering)
                d->commitBufferingTask(false);
        } else {
            d->src = src;
            d->takeOwnership = takeOwnership;
        }
    }

    /**
     * Gets the source from which to read from.
     */
    PositionableAudioSource *BufferingAudioSource::source() const {
        Q_D(const BufferingAudioSource);
        return d->src;
    }

    /**
     * Gets a global thread pool (differs from the QThreadPool::globalInstance).
     */
    QThreadPool *BufferingAudioSource::threadPool() {
        static QThreadPool *globalThreadPool = nullptr;
        static QMutex globalMtx;
        QMutexLocker locker(&globalMtx);
        if (!globalThreadPool)
            globalThreadPool = new QThreadPool;
        return globalThreadPool;
    }

    BufferingAudioSource::BufferingAudioSource(BufferingAudioSourcePrivate &d) : PositionableAudioSource(d) {

    }

    /**
     * Waits for the current buffering task to finish.
     * @param deadline the deadline
     * @return whether the task is finish
     */
    bool BufferingAudioSource::waitForBuffering(QDeadlineTimer deadline) {
        Q_D(BufferingAudioSource);
        QMutexLocker locker(&d->bufferingTaskMutex);
        return d->bufferingFinished.wait(&d->bufferingTaskMutex, deadline);
    }

    /**
     * Flushes the buffer.
     */
    void BufferingAudioSource::flush() {
        Q_D(BufferingAudioSource);
        QMutexLocker locker(&d->mutex);
        d->terminateCurrentBufferingTask();
        d->src->setNextReadPosition(d->position);
        QMutexLocker bufLocker(&d->bufLock);
        d->buf.clear();
        d->headPosition = 0;
        d->tailPosition = 0;
    }

    BufferingAudioSourceTask::BufferingAudioSourceTask(BufferingAudioSourcePrivate *d) : d(d) {
    }

    void BufferingAudioSourceTask::run() {
        qint64 head = d->headPosition;
        qint64 tail = d->tailPosition;
        if (head > d->readAheadSize) {
            QMutexLocker locker(&d->bufLock);
            head = d->headPosition;
            tail = d->tailPosition;
            for (int ch = 0; ch < d->channelCount; ch++) {
                d->buf.setSampleRange(ch, 0, tail - head, d->buf, ch, head);
            }
            d->tailPosition -= head;
            d->headPosition = 0;
            head = d->headPosition;
            tail = d->tailPosition;
        }
        readByFrame(tail, d->readAheadSize - (tail - head));
        {
            QMutexLocker locker(&d->bufferingTaskMutex);
            d->currentBufferingTask = nullptr;
            d->bufferingFinished.wakeAll();
        }
    }

    void BufferingAudioSourceTask::readByFrame(qint64 startPos, qint64 length) const {
        if (d->src->length() - d->src->nextReadPosition() < length) {
            for (int ch = 0; ch < d->channelCount; ch++)
                d->buf.clear(ch, startPos + d->src->length() - d->src->nextReadPosition(), length + d->src->nextReadPosition() - d->src->length());
            length = d->src->length() - d->src->nextReadPosition();
        }
        if (length <= 0)
            return;
        qint64 frameLength = qMin(d->src->bufferSize(), length);
        length = length - (length % frameLength);
        for (qint64 offset = 0; offset < length; offset += frameLength) {
            if (d->isTerminateRequested)
                return;
            d->src->read(AudioSourceReadData(&d->buf, startPos + offset, frameLength));
            d->tailPosition += frameLength;
        }
    }


    void BufferingAudioSourcePrivate::commitBufferingTask(bool isCritical) {
        currentBufferingTask = new BufferingAudioSourceTask(this);
        if (isCritical) {
            QScopedPointer<QRunnable> p(currentBufferingTask);
            currentBufferingTask->run();
        } else {
            threadPool->start(currentBufferingTask);
        }
    }

    void BufferingAudioSourcePrivate::terminateCurrentBufferingTask() {
        Q_Q(BufferingAudioSource);
        if (threadPool->tryTake(currentBufferingTask)) {
            delete currentBufferingTask;
            currentBufferingTask = nullptr;
            return;
        }
        {
            QMutexLocker locker(&bufferingTaskMutex);
            if (!currentBufferingTask)
                return;
            isTerminateRequested = true;
            bufferingFinished.wait(&bufferingTaskMutex);
            isTerminateRequested = false;
        }
    }

    void BufferingAudioSourcePrivate::accelerateCurrentBufferingTaskAndWait() {
        Q_Q(BufferingAudioSource);
        terminateCurrentBufferingTask();
        commitBufferingTask(true);
    }


} // talcs