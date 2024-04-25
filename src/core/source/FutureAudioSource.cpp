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

#include "FutureAudioSource.h"
#include "FutureAudioSource_p.h"

#include <QDebug>

namespace talcs {

    void FutureAudioSourcePrivate::_q_statusChanged(FutureAudioSource::Status status) {
        Q_Q(FutureAudioSource);
        if (status == FutureAudioSource::Ready) {
            src = futureWatcher->result();
            if (q->isOpen()) {
                Q_ASSERT(src->open(q->bufferSize(), q->sampleRate()));
                src->setNextReadPosition(position);
            }
        }
        emit q->statusChanged(status);
    }

    /**
     * @class FutureAudioSource
     * @brief The class takes an PositionableAudioSource object as the result of a asynchronous process.
     *
     * The PositionableAudioSource is the result of a asynchronous process (i.e. synthesizing, loading from the Internet) and
     * will be ready at a later point of time. [QFuture](https://doc.qt.io/qt-5/qfuture.html) is used for the implementation
     * of this class. (But the Qt Concurrent framework is not used, only [QFuture](https://doc.qt.io/qt-5/qfuture.html)
     * and other relevant classes are used)
     *
     * For functionality and compatibility of both Qt 5 and 6, This object requires some restrictions on the
     * [QFuture](https://doc.qt.io/qt-5/qfuture.html) object:
     *
     * - The [QFuture::progressMinimum](https://doc.qt.io/qt-5/qfuture.html#progressMinimum)() should be set to 0 and the
     * [QFuture::progressMaximum](https://doc.qt.io/qt-5/qfuture.html#progressMaximum)() should be set to the length of
     * the source.
     *
     * - It should be started before used by this object.
     *
     * - There should be only one result in it. Before finished, the [QFuture::progressValue](https://doc.qt.io/qt-5/qfuture.html#progressValue)()
     * should be set to the maximum and the result should be reported.
     *
     * - Its progress should be incremental.
     *
     * - Once finished or cancelled, it should not be started again.
     *
     * @see [QFuture](https://doc.qt.io/qt-5/qfuture.html), [QFutureWatcher](https://doc.qt.io/qt-5/qfuturewatcher.html), QFutureInterface
     */

    /**
     * @class FutureAudioSource::Callbacks
     * @brief The callback functions for the FutureAudioSource object.
     *
     * Note that:
     *
     * - Preloading callback functions could be invoked just after the source is ready, which should be handled properly.
     *
     * - If this object is opened while the source is still loading, the source will be automatically opened once loading
     * finished. Therefore, the preloading open callback function does not need to handle such case.
     *
     * @var FutureAudioSource::Callbacks::preloadingOpen
     * This function is invoked when open() is called before the source is ready.
     *
     * @var FutureAudioSource::Callbacks::preloadingClose
     * This function is invoked when close() is called before the source is ready.
     */

    /**
     * Constructor.
     * @param future the [QFuture](https://doc.qt.io/qt-5/qfuture.html) object to be used.
     * @param callbacks the callbacks to be invoked when the object is being prepared.
     */
    FutureAudioSource::FutureAudioSource(const QFuture<PositionableAudioSource *> &future,
                                         const Callbacks &callbacks, QObject *parent)
        : QObject(parent), PositionableAudioSource(*new FutureAudioSourcePrivate) {
        Q_D(FutureAudioSource);
        d->callbacks = callbacks;
        d->futureWatcher = new QFutureWatcher<PositionableAudioSource *>(this);
        connect(d->futureWatcher, &QFutureWatcher<PositionableAudioSource *>::paused, this, [=]() { d->_q_statusChanged(Paused); });
        connect(d->futureWatcher, &QFutureWatcher<PositionableAudioSource *>::resumed, this, [=]() { d->_q_statusChanged(Running); });
        connect(d->futureWatcher, &QFutureWatcher<PositionableAudioSource *>::canceled, this,
                [=]() { d->_q_statusChanged(Cancelled); });
        connect(d->futureWatcher, &QFutureWatcher<PositionableAudioSource *>::finished, this, [=]() { d->_q_statusChanged(Ready); });
        connect(d->futureWatcher, &QFutureWatcher<PositionableAudioSource *>::progressValueChanged, this,
                [=](int progressValue) { emit progressChanged(progressValue); });
        d->futureWatcher->setFuture(future);
    }

    /**
     * Destructor.
     */
    FutureAudioSource::~FutureAudioSource() {
        Q_D(FutureAudioSource);
        FutureAudioSource::close();
        disconnect(d->futureWatcher, nullptr, this, nullptr);
    }

    /**
     * Gets the [QFuture](https://doc.qt.io/qt-5/qfuture.html) object that is used.
     */
    QFuture<PositionableAudioSource *> FutureAudioSource::future() const {
        Q_D(const FutureAudioSource);
        return d->futureWatcher->future();
    }

    void FutureAudioSource::setFuture(const QFuture<PositionableAudioSource *> &future) {
        Q_D(FutureAudioSource);
        QMutexLocker locker(&d->mutex);
        if (isOpen()) {
            if (!d->callbacks.preloadingOpen(bufferSize(), sampleRate())) {
                qWarning() << "FutureAudioSource: Preloading open callback fails when setting future.";
                return;
            }
        }
        d->src = nullptr;
        d->futureWatcher->setFuture(future);
    }

    qint64 FutureAudioSource::processReading(const AudioSourceReadData &readData) {
        Q_D(FutureAudioSource);
        QMutexLocker locker(&d->mutex);
        if (d->src) {
            auto readLength = d->src->read(readData);
            d->position += readLength;
            return readLength;
        } else {
            for (int ch = 0; ch < readData.buffer->channelCount(); ch++) {
                readData.buffer->clear(ch, readData.startPos, readData.length);
            }
            auto readLength = qMin(readData.length, length() - d->position);
            d->position += readLength;
            return readLength;
        }
    }

    qint64 FutureAudioSource::length() const {
        Q_D(const FutureAudioSource);
        return d->futureWatcher->progressMaximum();
    }

    qint64 FutureAudioSource::nextReadPosition() const {
        return PositionableAudioSource::nextReadPosition();
    }

    void FutureAudioSource::setNextReadPosition(qint64 pos) {
        Q_D(FutureAudioSource);
        QMutexLocker locker(&d->mutex);
        PositionableAudioSource::setNextReadPosition(pos);
    }

    bool FutureAudioSource::open(qint64 bufferSize, double sampleRate) {
        Q_D(FutureAudioSource);
        QMutexLocker locker(&d->mutex);
        switch (status()) {
            case Running:
            case Paused:
                return d->callbacks.preloadingOpen(bufferSize, sampleRate) &&
                       AudioSource::open(bufferSize, sampleRate);
            case Cancelled:
                return false;
            case Ready:
                if (d->src->open(bufferSize, sampleRate)) {
                    d->src->setNextReadPosition(d->position);
                    return AudioSource::open(bufferSize, sampleRate);
                }
                return false;
            default:
                return false;
        }
    }

    void FutureAudioSource::close() {
        Q_D(FutureAudioSource);
        QMutexLocker locker(&d->mutex);
        switch (status()) {
            case Running:
            case Paused:
                d->callbacks.preloadingClose();
                break;
            case Cancelled:
                break;
            case Ready:
                source()->close();
                break;
        }
        talcs::AudioSource::close();
    }

    /**
     * Gets the progress of preparation.
     */
    int FutureAudioSource::progress() const {
        Q_D(const FutureAudioSource);
        return d->futureWatcher->progressValue();
    }

    /**
     * Pauses the preparation.
     */
    void FutureAudioSource::pause() {
        Q_D(FutureAudioSource);
        d->futureWatcher->pause();
    }

    /**
     * Resumes the preparation.
     */
    void FutureAudioSource::resume() {
        Q_D(FutureAudioSource);
        d->futureWatcher->resume();
    }

    /**
     * Cancels the preparation.
     */
    void FutureAudioSource::cancel() {
        Q_D(FutureAudioSource);
        d->futureWatcher->cancel();
    }

    /**
     * @enum FutureAudioSource::Status
     * The status of preparation.
     *
     * @var FutureAudioSource::Running
     * The preparation is being processed.
     *
     * @var FutureAudioSource::Paused
     * The preparation is paused.
     *
     * @var FutureAudioSource::Cancelled
     * The preparation is cancelled.
     *
     * @var FutureAudioSource::Ready
     * The source is ready for read.
     */

    /**
     * Gets the status.
     */
    FutureAudioSource::Status FutureAudioSource::status() const {
        Q_D(const FutureAudioSource);
        if (d->futureWatcher->isCanceled())
            return Cancelled;
        if (d->futureWatcher->isPaused())
            return Paused;
        if (d->futureWatcher->isFinished())
            return Ready;
        return Running;
    }

    /**
     * Waits for the preparation to finish.
     */
    void FutureAudioSource::wait() {
        Q_D(FutureAudioSource);
        d->futureWatcher->waitForFinished();
    }

    /**
     * Gets the source object.
     *
     * If the source is not ready, @c nullptr will be returned.
     */
    PositionableAudioSource *FutureAudioSource::source() const {
        Q_D(const FutureAudioSource);
        return d->src;
    }

    /**
     * @fn void FutureAudioSource::statusChanged(Status status)
     * Emitted when the status is changed.
     */

    /**
     * @fn void FutureAudioSource::progressChanged(int progress)
     * Emitted when the progress value is changed.
     */
     
}