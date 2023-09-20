#include "FutureAudioSource.h"
#include "FutureAudioSource_p.h"

namespace talcs {

    void FutureAudioSourcePrivate::_q_statusChanged(FutureAudioSource::Status status) {
        Q_Q(FutureAudioSource);
        if (status == FutureAudioSource::Ready) {
            src = futureWatcher.result();
            if(q->isOpen()) {
                Q_ASSERT(src->open(q->bufferSize(), q->sampleRate()));
                src->setNextReadPosition(position);
            }
        }
        emit q->statusChanged(status);
    }

    FutureAudioSource::FutureAudioSource(const QFuture<PositionableAudioSource *> &future,
                                         const FutureAudioSourceCallbacks &callbacks): PositionableAudioSource(*new FutureAudioSourcePrivate) {
        Q_D(FutureAudioSource);
        d->futureWatcher.setFuture(future);
        d->callbacks = callbacks;
        d->mutex = future.d.mutex();
        connect(&d->futureWatcher, &decltype(d->futureWatcher)::paused, this, [=](){
            d->_q_statusChanged(Paused);
        });
        connect(&d->futureWatcher, &decltype(d->futureWatcher)::resumed, this, [=](){
            d->_q_statusChanged(Running);
        });
        connect(&d->futureWatcher, &decltype(d->futureWatcher)::canceled, this, [=](){
            d->_q_statusChanged(Cancelled);
        });
        connect(&d->futureWatcher, &decltype(d->futureWatcher)::finished, this, [=](){
            d->_q_statusChanged(Ready);
        });
        connect(&d->futureWatcher, &decltype(d->futureWatcher)::progressValueChanged, this, [=](int progressValue){
            emit progressChanged(progressValue);
        });
    }

    FutureAudioSource::~FutureAudioSource() = default;

    QFuture<PositionableAudioSource *> FutureAudioSource::future() const {
        Q_D(const FutureAudioSource);
        return d->futureWatcher.future();
    }

    qint64 FutureAudioSource::read(const AudioSourceReadData &readData) {
        Q_D(FutureAudioSource);
        QMutexLocker locker(d->mutex);
        if (d->src) {
            auto readLength = d->src->read(readData);
            d->position += readLength;
            return readLength;
        } else {
            for (int ch = 0; ch < readData.buffer->channelCount(); ch++) {
                readData.buffer->clear(ch, readData.startPos, readData.length);
            }
            auto readLength = std::min(readData.length, length() - d->position);
            d->position += readLength;
            return readLength;
        }
    }

    qint64 FutureAudioSource::length() const {
        Q_D(const FutureAudioSource);
        return d->futureWatcher.progressMaximum();
    }

    qint64 FutureAudioSource::nextReadPosition() const {
        return PositionableAudioSource::nextReadPosition();
    }

    void FutureAudioSource::setNextReadPosition(qint64 pos) {
        Q_D(FutureAudioSource);
        QMutexLocker locker(d->mutex);
        PositionableAudioSource::setNextReadPosition(pos);
    }

    bool FutureAudioSource::open(qint64 bufferSize, double sampleRate) {
        Q_D(FutureAudioSource);
        QMutexLocker locker(d->mutex);
        switch (status()) {
            case Running:
                return d->callbacks.preloadingOpen(bufferSize, sampleRate) && AudioStreamBase::open(bufferSize, sampleRate);
            case Paused:
                return d->callbacks.preloadingOpen(bufferSize, sampleRate) && AudioStreamBase::open(bufferSize, sampleRate);
            case Cancelled:
                return false;
            case Ready:
                if(d->src->open(bufferSize, sampleRate)) {
                    d->src->setNextReadPosition(d->position);
                    return AudioStreamBase::open(bufferSize, sampleRate);
                }
                return false;
        }
    }

    void FutureAudioSource::close() {
        Q_D(FutureAudioSource);
        QMutexLocker locker(d->mutex);
        switch (status()) {
            case Running:
                d->callbacks.preloadingClose();
                break;
            case Paused:
                d->callbacks.preloadingClose();
                break;
            case Cancelled:
                break;
            case Ready:
                source()->close();
                break;
        }
        AudioStreamBase::close();
    }

    int FutureAudioSource::progress() const {
        Q_D(const FutureAudioSource);
        return d->futureWatcher.progressValue();
    }

    void FutureAudioSource::pause() {
        Q_D(FutureAudioSource);
        d->futureWatcher.pause();
    }

    void FutureAudioSource::resume() {
        Q_D(FutureAudioSource);
        d->futureWatcher.resume();
    }

    void FutureAudioSource::cancel() {
        Q_D(FutureAudioSource);
        d->futureWatcher.cancel();
    }

    FutureAudioSource::Status FutureAudioSource::status() const {
        Q_D(const FutureAudioSource);
        if(d->futureWatcher.isRunning())
            return Running;
        if(d->futureWatcher.isPaused())
            return Paused;
        if(d->futureWatcher.isFinished())
            return Ready;
        return Cancelled;
    }

    void FutureAudioSource::wait() {
        Q_D(FutureAudioSource);
        d->futureWatcher.waitForFinished();
    }

    PositionableAudioSource *FutureAudioSource::source() const {
        Q_D(const FutureAudioSource);
        return d->src;
    }
} // talcs