#include "PositionableMixerAudioSource.h"
#include "PositionableMixerAudioSource_p.h"

#include "buffer/AudioBuffer.h"

namespace talcs {

    /**
     * @class PositionableMixerAudioSource
     * @brief A class very similar to MixerAudioSource, but this one supports repositioning.
     * @see MixerAudioSource
     */

    /**
     * @copydoc MixerAudioSource::MixerAudioSource()
     */
    PositionableMixerAudioSource::PositionableMixerAudioSource(QObject *parent)
        : PositionableMixerAudioSource(*new PositionableMixerAudioSourcePrivate, parent) {
    }
    PositionableMixerAudioSource::PositionableMixerAudioSource(PositionableMixerAudioSourcePrivate & d, QObject *parent)
        : QObject(parent), PositionableAudioSource(d) {
    }

    /**
     * @copydoc MixerAudioSource::~MixerAudioSource()
     */
    PositionableMixerAudioSource::~PositionableMixerAudioSource() {
        Q_D(PositionableMixerAudioSource);
        PositionableMixerAudioSource::close();
        d->deleteOwnedSources();
    }

    /**
     * @copydoc MixerAudioSource::open()
     *
     * All input sources are set to the same position after open.
     */
    bool PositionableMixerAudioSource::open(qint64 bufferSize, double sampleRate) {
        Q_D(PositionableMixerAudioSource);
        QMutexLocker locker(&d->mutex);
        d->stop();
        if (d->start(bufferSize, sampleRate)) {
            d->setNextReadPositionToAll(d->position);
            return PositionableAudioSource::open(bufferSize, sampleRate);
        } else {
            return false;
        }
    }

    qint64 PositionableMixerAudioSource::read(const AudioSourceReadData &readData) {
        Q_D(PositionableMixerAudioSource);
        QList<float> magnitude;
        qint64 readLength;
        {
            QMutexLocker locker(&d->mutex);
            auto bufferLength = length();
            auto channelCount = readData.buffer->channelCount();
            if (d->tmpBuf.channelCount() < channelCount)
                d->tmpBuf.resize(channelCount);
            for (int i = 0; i < channelCount; i++) {
                readData.buffer->clear(i, readData.startPos, readData.length);
            }
            readLength = std::min(readData.length, bufferLength - nextReadPosition());
            d->mix(readData, readLength);

            for (int i = 0; i < channelCount; i++) {
                magnitude.append(readData.buffer->magnitude(i, readData.startPos, readLength));
            }
            d->position += readLength;
        }

        float magL = 0, magR = 0;
        if (magnitude.length() >= 2) {
            magL = magnitude[0];
            magR = magnitude[1];
        } else if (magnitude.length() == 1) {
            magL = magR = magnitude[0];
        }
        emit meterUpdated(magL, magR);
        return readLength;
    }

    /**
     * @copydoc MixerAudioSource::close()
     */
    void PositionableMixerAudioSource::close() {
        Q_D(PositionableMixerAudioSource);
        QMutexLocker locker(&d->mutex);
        d->tmpBuf.resize(0, 0);
        PositionableAudioSource::close();
    }

    /**
     * Returns the minimum length among all input sources.
     */
    qint64 PositionableMixerAudioSource::length() const {
        auto sourceList = sources();
        if (sourceList.length() == 0)
            return 0;
        return (*std::min_element(sourceList.begin(), sourceList.end(),
                                  [](PositionableAudioSource *src1, PositionableAudioSource *src2) {
                                      return src1->length() < src2->length();
                                  }))
            ->length();
    }

    void PositionableMixerAudioSourcePrivate::setNextReadPositionToAll(qint64 pos) {
        std::for_each(sourceDict.constBegin(), sourceDict.constEnd(),
                      [=](const SourceInfo<PositionableAudioSource> &srcInfo) { srcInfo.src->setNextReadPosition(pos); });
    }

    /**
     * Sets the next read position, and updates the read position to all input sources.
     */
    void PositionableMixerAudioSource::setNextReadPosition(qint64 pos) {
        Q_D(PositionableMixerAudioSource);
        QMutexLocker locker(&d->mutex);
        d->setNextReadPositionToAll(pos);
        PositionableAudioSource::setNextReadPosition(pos);
    }

    bool PositionableMixerAudioSource::addSource(PositionableAudioSource * src, bool takeOwnership) {
        if (src == this)
            return false;
        Q_D(PositionableMixerAudioSource);
        QMutexLocker locker(&d->mutex);
        if (!d->addSource(src, takeOwnership, isOpen(), bufferSize(), sampleRate()))
            return false;
        if (isOpen()) {
            src->setNextReadPosition(nextReadPosition());
        }
        return true;
    }

    bool PositionableMixerAudioSource::removeSource(PositionableAudioSource * src) {
        Q_D(PositionableMixerAudioSource);
        QMutexLocker locker(&d->mutex);
        return d->removeSource(src);
    }

    void PositionableMixerAudioSource::removeAllSources() {
        Q_D(PositionableMixerAudioSource);
        QMutexLocker locker(&d->mutex);
        d->removeAllSources();
    }

    QList<PositionableAudioSource *> PositionableMixerAudioSource::sources() const {
        Q_D(const PositionableMixerAudioSource);
        return d->sources();
    }

    void PositionableMixerAudioSource::setSourceSolo(PositionableAudioSource *src, bool isSolo) {
        Q_D(PositionableMixerAudioSource);
        QMutexLocker locker(&d->mutex);
        d->setSourceSolo(src, isSolo);
    }

    bool PositionableMixerAudioSource::isSourceSolo(PositionableAudioSource *src) const {
        Q_D(const PositionableMixerAudioSource);
        return d->isSourceSolo(src);
    }

    void PositionableMixerAudioSource::setGain(float gain) {
        Q_D(PositionableMixerAudioSource);
        QMutexLocker locker(&d->mutex);
        d->gain = gain;
    }

    float PositionableMixerAudioSource::gain() const {
        Q_D(const PositionableMixerAudioSource);
        return d->gain;
    }

    void PositionableMixerAudioSource::setPan(float pan) {
        Q_D(PositionableMixerAudioSource);
        QMutexLocker locker(&d->mutex);
        d->pan = pan;
    }

    float PositionableMixerAudioSource::pan() const {
        Q_D(const PositionableMixerAudioSource);
        return d->pan;
    }

    void PositionableMixerAudioSource::setRouteChannels(bool routeChannels) {
        Q_D(PositionableMixerAudioSource);
        QMutexLocker locker(&d->mutex);
        d->routeChannels = routeChannels;
    }

    bool PositionableMixerAudioSource::routeChannels() const {
        Q_D(const PositionableMixerAudioSource);
        return d->routeChannels;
    }

    void PositionableMixerAudioSource::setSilentFlags(int silentFlags) {
        Q_D(PositionableMixerAudioSource);
        QMutexLocker locker(&d->mutex);
        d->silentFlags = silentFlags;
    }

    int PositionableMixerAudioSource::silentFlags() const {
        Q_D(const PositionableMixerAudioSource);
        return d->silentFlags;
    }
    
}
