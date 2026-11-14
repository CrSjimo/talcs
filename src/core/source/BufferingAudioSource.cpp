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
     * @param readAheadSize the buffering size. Reads go directly to the source if this size is at most bufferSize().
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
            const auto readLength = d->src->read(readData);
            d->position += readLength;
            return readLength;
        }
        const auto channelCount = qMin(readData.buffer->channelCount(), d->channelCount);
        qint64 readLength = 0;
        while (readLength < readData.length) {
            const auto chunkLength = qMin(readData.length - readLength, d->readAheadSize);
            QMutexLocker readLocker(&d->bufLock);
            if (d->tailPosition.loadAcquire() - d->headPosition < chunkLength) {
                readLocker.unlock();
                d->terminateCurrentBufferingTask();
                d->fillBuffer(chunkLength);
                readLocker.relock();
            }
            const auto available = qMin(chunkLength, d->tailPosition.loadAcquire() - d->headPosition);
            if (available > 0) {
                for (int ch = 0; ch < channelCount; ch++) {
                    readData.buffer->setSampleRange(ch, readData.startPos + readLength, available, d->buf, ch, d->headPosition);
                }
                d->headPosition += available;
                readLength += available;
            }
            if (available < chunkLength)
                break;
        }
        for (int ch = 0; ch < channelCount; ch++) {
            readData.buffer->clear(ch, readData.startPos + readLength, readData.length - readLength);
        }
        for (int ch = channelCount; ch < readData.buffer->channelCount(); ch++) {
            readData.buffer->clear(ch, readData.startPos, readData.length);
        }
        d->position += readLength;
        if (readLength > 0 && readLength == readData.length)
            d->commitBufferingTask();
        return readLength;
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
                d->commitBufferingTask();
        } else {
            d->src->setNextReadPosition(pos);
        }
        PositionableAudioSource::setNextReadPosition(pos);
    }

    bool BufferingAudioSource::open(qint64 bufferSize, double sampleRate) {
        Q_D(BufferingAudioSource);
        QMutexLocker locker(&d->mutex);
        AudioSource::close();
        flush();
        if (!d->src->open(bufferSize, sampleRate))
            return false;
        d->src->setNextReadPosition(d->position);
        if (d->readAheadSize > bufferSize) {
            d->buf.resize(d->channelCount, d->readAheadSize * 2);
            if (d->autoBuffering)
                d->commitBufferingTask();
        }
        return AudioSource::open(bufferSize, sampleRate);
    }

    void BufferingAudioSource::close() {
        Q_D(BufferingAudioSource);
        QMutexLocker locker(&d->mutex);
        flush();
        AudioSource::close();
    }

    /**
     * Sets the buffering size.
     */
    void BufferingAudioSource::setReadAheadSize(qint64 size) {
        Q_D(BufferingAudioSource);
        QMutexLocker locker(&d->mutex);
        if (size == d->readAheadSize)
            return;
        if (isOpen() && (d->readAheadSize > bufferSize() || size > bufferSize()))
            flush();
        d->readAheadSize = size;
        if (isOpen() && size > bufferSize()) {
            d->buf.resize(d->channelCount, size * 2);
            if (d->autoBuffering)
                d->commitBufferingTask();
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
            d->buf.resize(channelCount, d->readAheadSize * 2);
            if (d->autoBuffering)
                d->commitBufferingTask();
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
                d->commitBufferingTask();
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
     * Waits for the current buffering task to finish or be canceled.
     * @param deadline the deadline
     * @return Whether the task finished or was canceled by the deadline, or no task was pending.
     */
    bool BufferingAudioSource::waitForBuffering(QDeadlineTimer deadline) {
        Q_D(BufferingAudioSource);
        QMutexLocker locker(&d->bufferingTaskMutex);
        if (!d->bufferingTaskPending)
            return true;
        const auto completedTasks = d->completedBufferingTasks;
        while (d->completedBufferingTasks == completedTasks) {
            if (!d->bufferingFinished.wait(&d->bufferingTaskMutex, deadline))
                return d->completedBufferingTasks != completedTasks;
        }
        return true;
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
        setAutoDelete(false);
    }

    void BufferingAudioSourceTask::run() {
        d->fillBuffer(d->readAheadSize);
        QMutexLocker locker(&d->bufferingTaskMutex);
        d->finishBufferingTask();
    }

    void BufferingAudioSourcePrivate::fillBuffer(qint64 targetLength) {
        qint64 missingLength;
        {
            QMutexLocker locker(&bufLock);
            const auto tail = tailPosition.loadRelaxed();
            const auto available = tail - headPosition;
            if (headPosition > readAheadSize) {
                if (available > 0) {
                    for (int ch = 0; ch < channelCount; ch++) {
                        buf.setSampleRange(ch, 0, available, buf, ch, headPosition);
                    }
                }
                headPosition = 0;
                tailPosition = available;
            }
            missingLength = qMax(0ll, targetLength - available);
        }

        auto remaining = qBound(0ll, src->length() - src->nextReadPosition(), missingLength);
        auto tail = tailPosition.loadRelaxed();
        while (remaining > 0 && !isTerminateRequested) {
            const auto frameLength = qMin(src->bufferSize(), remaining);
            const auto readLength = src->read({&buf, tail, frameLength});
            tail += readLength;
            tailPosition.storeRelease(tail);
            if (readLength < frameLength)
                break;
            remaining -= readLength;
        }
    }

    void BufferingAudioSourcePrivate::commitBufferingTask() {
        QMutexLocker locker(&bufferingTaskMutex);
        if (bufferingTaskPending)
            return;
        bufferingTaskPending = true;
        threadPool->start(&bufferingTask);
    }

    void BufferingAudioSourcePrivate::finishBufferingTask() {
        bufferingTaskPending = false;
        ++completedBufferingTasks;
        bufferingFinished.wakeAll();
    }

    void BufferingAudioSourcePrivate::terminateCurrentBufferingTask() {
        QMutexLocker locker(&bufferingTaskMutex);
        if (!bufferingTaskPending)
            return;
        if (threadPool->tryTake(&bufferingTask)) {
            finishBufferingTask();
            return;
        }
        isTerminateRequested = true;
        while (bufferingTaskPending)
            bufferingFinished.wait(&bufferingTaskMutex);
        isTerminateRequested = false;
    }


} // talcs
