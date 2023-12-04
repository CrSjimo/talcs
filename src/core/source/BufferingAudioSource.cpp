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

        d->buf.resize(channelCount, readAheadSize * 2);
        d->headPosition = 0;
        d->tailPosition = 0;
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
        {
            QMutexLocker readLocker(&d->bufLock);
            qint64 head = d->headPosition;
            qint64 tail = d->tailPosition;
            if (tail - head >= readData.length) {
                readFromBuffer:
                for (int ch = 0; ch < channelCount; ch++) {
                    readData.buffer->setSampleRange(ch, readData.startPos, readData.length, d->buf, ch, head);
                }
            } else {
                readLocker.unlock();
                d->accelerateCurrentBufferingTaskAndWait();
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
            d->terminateCurrentBufferingTask();
            d->buf.clear();
            d->src->setNextReadPosition(pos);
            d->headPosition = 0;
            d->tailPosition = 0;
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
            d->tailPosition = 0;
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
            d->buf.resize(-1, size * 2);
            d->headPosition = 0;
            d->tailPosition = 0;
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
        if (head > d->readAheadSize) {
            QMutexLocker locker(&d->bufLock);
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
        if (length <= 0)
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
            d->tailPosition += std::min(frameLength, length - offset);
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
        if (threadPool->tryTake(currentBufferingTask)) {
            currentBufferingTask = nullptr;
            return;
        }
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
            currentBufferingTask = nullptr;
            commitBufferingTask(true);
        } else {
            q->waitForBuffering();
            commitBufferingTask(true);
        }
    }


} // talcs