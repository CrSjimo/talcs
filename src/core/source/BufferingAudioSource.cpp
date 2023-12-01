#include "BufferingAudioSource.h"
#include "BufferingAudioSource_p.h"

#include <QThreadPool>

namespace talcs {

    BufferingAudioSource::BufferingAudioSource(PositionableAudioSource *src, int channelCount, qint64 readAheadSize,
                                               QThreadPool *threadPool) : BufferingAudioSource(src, false, channelCount, readAheadSize, threadPool) {

    }

    BufferingAudioSource::BufferingAudioSource(PositionableAudioSource *src, bool takeOwnership, int channelCount,
                                               qint64 readAheadSize, QThreadPool *threadPool) : BufferingAudioSource(*new BufferingAudioSourcePrivate) {
        Q_D(BufferingAudioSource);
        d->src = src;
        d->takeOwnership = takeOwnership;
        d->channelCount = channelCount;
        d->readAheadSize = readAheadSize;
        d->threadPool = threadPool ? threadPool : BufferingAudioSource::threadPool();

        d->buf.resize(channelCount, readAheadSize);
        d->headPosition = 0;
        d->tailPosition = 1;
    }

    BufferingAudioSource::~BufferingAudioSource() {
        BufferingAudioSource::close();
    }

    qint64 BufferingAudioSource::read(const AudioSourceReadData &readData) {
        Q_D(BufferingAudioSource);
        QMutexLocker locker(&d->mutex);
        Q_ASSERT(isOpen());
        if (d->readAheadSize <= bufferSize()) {
            return d->src->read(readData);
        }
        int channelCount = std::min(readData.buffer->channelCount(), d->channelCount);
        qint64 head = d->headPosition;
        qint64 tail = d->tailPosition;
        if (tail > head && tail - head > readData.length) {
            for (int ch = 0; ch < channelCount; ch++) {
                readData.buffer->setSampleRange(ch, readData.startPos, readData.length, d->buf, ch, head);
            }
        } else if (tail <= head && d->readAheadSize - head >= readData.length) {
            for (int ch = 0; ch < channelCount; ch++) {
                readData.buffer->setSampleRange(ch, readData.startPos, readData.length, d->buf, ch, head);
            }

        } else if (tail <= head && (head + readData.length) % d->readAheadSize <= tail) {
            for (int ch = 0; ch < channelCount; ch++) {
                readData.buffer->setSampleRange(ch, readData.startPos, d->readAheadSize - head, d->buf, ch, head);
                readData.buffer->setSampleRange(ch, readData.startPos + d->readAheadSize - head, readData.length + head - d->readAheadSize, d->buf, ch, 0);
            }
        } else {
            d->accelerateCurrentBufferingTaskAndWait();
            Q_ASSERT(head == d->tailPosition);
            if (d->readAheadSize - head >= readData.length) {
                for (int ch = 0; ch < channelCount; ch++) {
                    readData.buffer->setSampleRange(ch, readData.startPos, readData.length, d->buf, ch, head);
                }
            } else {
                for (int ch = 0; ch < channelCount; ch++) {
                    readData.buffer->setSampleRange(ch, readData.startPos, d->readAheadSize - head, d->buf, ch, head);
                    readData.buffer->setSampleRange(ch, readData.startPos + d->readAheadSize - head, readData.length + head - d->readAheadSize, d->buf, ch, 0);
                }
            }
        }
        d->headPosition = (head + readData.length) % d->readAheadSize;
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
            d->terminateCurrentBufferingTask();
            d->buf.clear();
            d->src->setNextReadPosition(pos);
            d->headPosition = 0;
            d->tailPosition = 1;
            d->commitBufferingTask(false);
        }
        PositionableAudioSource::setNextReadPosition(pos);
    }

    bool BufferingAudioSource::open(qint64 bufferSize, double sampleRate) {
        Q_D(BufferingAudioSource);
        QMutexLocker locker(&d->mutex);
        d->terminateCurrentBufferingTask();
        if (!d->src->open(bufferSize, sampleRate))
            return false;
        if (d->readAheadSize > bufferSize) {
            d->buf.clear();
            d->headPosition = 0;
            d->tailPosition = 1;
            d->commitBufferingTask(false);
        }
        return AudioStreamBase::open(bufferSize, sampleRate);
    }

    void BufferingAudioSource::close() {
        Q_D(BufferingAudioSource);
        QMutexLocker locker(&d->mutex);
        d->terminateCurrentBufferingTask();
    }

    void BufferingAudioSource::setReadAheadSize(qint64 size) {
        Q_D(BufferingAudioSource);
        QMutexLocker locker(&d->mutex);
        if (size == d->readAheadSize)
            return;
        if (isOpen() && size > bufferSize()) {
            d->terminateCurrentBufferingTask();
            d->buf.clear();
            d->buf.resize(-1, size);
            d->headPosition = 0;
            d->tailPosition = 1;
            d->commitBufferingTask(false);
        }
        d->readAheadSize = size;
    }

    qint64 BufferingAudioSource::readAheadSize() const {
        Q_D(const BufferingAudioSource);
        return d->readAheadSize;
    }

    PositionableAudioSource *BufferingAudioSource::source() const {
        Q_D(const BufferingAudioSource);
        return d->src;
    }

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

    bool BufferingAudioSource::waitForBuffering(QDeadlineTimer deadline) {
        Q_D(BufferingAudioSource);
        QMutexLocker locker(&d->bufferingTaskMutex);
        return d->bufferingFinished.wait(&d->bufferingTaskMutex, deadline);
    }

    BufferingAudioSourceTask::BufferingAudioSourceTask(BufferingAudioSourcePrivate *d) : d(d) {
    }

    void BufferingAudioSourceTask::run() {
        qint64 head = d->headPosition;
        qint64 tail = d->tailPosition;
        if (tail > head) {
            readByFrame(tail, d->readAheadSize - tail);
            readByFrame(0, head);
        } else {
            readByFrame(tail, head - tail);
        }
        d->tailPosition = head;
        {
            QMutexLocker locker(&d->bufferingTaskMutex);
            d->currentBufferingTask = nullptr;
            d->bufferingFinished.wakeAll();
        }
    }

    void BufferingAudioSourceTask::readByFrame(qint64 startPos, qint64 length) const {
        if (length == 0)
            return;
        if (d->src->length() - d->src->nextReadPosition() < length) {
            for (int ch = 0; ch < d->channelCount; ch++)
                d->buf.clear(ch, startPos + d->src->length() - d->src->nextReadPosition(), length + d->src->nextReadPosition() - d->src->length());
            length = d->src->length() - d->src->nextReadPosition();
        }
        qint64 frameLength = d->src->bufferSize();
        for (qint64 offset = 0; offset < length; offset += frameLength) {
            if (d->isTerminateRequested)
                return;
            d->src->read(AudioSourceReadData(&d->buf, startPos + offset, std::min(frameLength, length - offset)));
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
        if (!currentBufferingTask)
            return;
        if (threadPool->tryTake(currentBufferingTask))
            return;
        isTerminateRequested = true;
        q->waitForBuffering();
        isTerminateRequested = false;
    }

    void BufferingAudioSourcePrivate::accelerateCurrentBufferingTaskAndWait() {
        Q_Q(BufferingAudioSource);
        if (!currentBufferingTask) {
            commitBufferingTask(true);
        } else if (threadPool->tryTake(currentBufferingTask)) {
            delete currentBufferingTask;
            commitBufferingTask(true);
        } else {
            q->waitForBuffering();
        }
    }


} // talcs