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

#include "PositionableMixerAudioSource.h"
#include "PositionableMixerAudioSource_p.h"

#include "buffer/AudioBuffer.h"

namespace talcs {

    /**
     * @class PositionableMixerAudioSource
     * @brief A class very similar to MixerAudioSource, but this one supports repositioning.
     *
     * This is one of the @ref doc/object_binding.md "object-binding" classes.
     *
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
        qint64 readLength;
        auto channelCount = readData.buffer->channelCount();
        {
            QMutexLocker locker(&d->mutex);
            auto bufferLength = length();
            if (d->tmpBuf.channelCount() < channelCount)
                d->tmpBuf.resize(channelCount);
            for (int i = 0; i < channelCount; i++) {
                readData.buffer->clear(i, readData.startPos, readData.length);
            }
            readLength = qMin(readData.length, bufferLength - nextReadPosition());
            d->mix(readData, readLength);
            d->position += readLength;
        }
        if (d->isMeterEnabled) {
            float magnitude[2] = {0, 0};
            for (int i = 0; i < channelCount; i++) {
                if (i > 1)
                    break;
                magnitude[i] = readData.buffer->magnitude(i, readData.startPos, readLength);
            }
            if (channelCount == 1) {
                magnitude[1] = magnitude[0];
            }
            emit meterUpdated(magnitude[0], magnitude[1]);
        }
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

    void PositionableMixerAudioSource::setMeterEnabled(bool enabled) {
        Q_D(PositionableMixerAudioSource);
        QMutexLocker locker(&d->mutex);
        d->isMeterEnabled = enabled;
    }

    bool PositionableMixerAudioSource::isMeterEnabled() const {
        Q_D(const PositionableMixerAudioSource);
        return d->isMeterEnabled;
    }

    /**
     * @fn void PositionableMixerAudioSource::meterUpdated(float leftMagnitude, float rightMagnitude)
     * Emitted on each block processed.
     *
     * To use this signal, setMeterEnabled() must be set to true.
     */

}
