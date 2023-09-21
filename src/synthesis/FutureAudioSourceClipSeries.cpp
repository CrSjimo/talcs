#include "FutureAudioSourceClipSeries.h"
#include "FutureAudioSourceClipSeries_p.h"

namespace talcs {
    FutureAudioSourceClipSeries::FutureAudioSourceClipSeries(QObject *parent) : QObject(parent), PositionableAudioSource(*new FutureAudioSourceClipSeriesPrivate) {
    }
    FutureAudioSourceClipSeries::~FutureAudioSourceClipSeries() {
        FutureAudioSourceClipSeries::close();
    }
    qint64 FutureAudioSourceClipSeries::read(const AudioSourceReadData &readData) {
        return 0;
    }
    qint64 FutureAudioSourceClipSeries::length() const {
        return 0;
    }
    qint64 FutureAudioSourceClipSeries::nextReadPosition() const {
        return PositionableAudioSource::nextReadPosition();
    }
    void FutureAudioSourceClipSeries::setNextReadPosition(qint64 pos) {
        PositionableAudioSource::setNextReadPosition(pos);
    }
    bool FutureAudioSourceClipSeries::addClip(const AudioClipBase<FutureAudioSource> &clip) {
        return AudioClipSeriesBase::addClip(clip);
    }
    bool FutureAudioSourceClipSeries::removeClipAt(qint64 pos) {
        return AudioClipSeriesBase::removeClipAt(pos);
    }
    void FutureAudioSourceClipSeries::clearClips() {
        AudioClipSeriesBase::clearClips();
    }
    bool FutureAudioSourceClipSeries::open(qint64 bufferSize, double sampleRate) {
        return AudioStreamBase::open(bufferSize, sampleRate);
    }
    void FutureAudioSourceClipSeries::close() {
        AudioStreamBase::close();
    }
    qint64 FutureAudioSourceClipSeries::lengthAvailableInTotal() const {
        return 0;
    }
    qint64 FutureAudioSourceClipSeries::lengthAvailableFrom(qint64 pos) const {
        return 0;
    }
    void FutureAudioSourceClipSeries::setReadMode(FutureAudioSourceClipSeries::ReadMode readMode) {
        Q_D(FutureAudioSourceClipSeries);
        QMutexLocker locker(&d->mutex);
        d->readMode = readMode;
    }
    FutureAudioSourceClipSeries::ReadMode FutureAudioSourceClipSeries::readMode() const {
        Q_D(const FutureAudioSourceClipSeries);
        return d->readMode;
    }
    bool FutureAudioSourceClipSeries::isCurrentPositionReady() const {
        return false;
    }

} // talcs