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
        qRegisterMetaType<QVector<float>>();
    }

    PositionableMixerAudioSource::PositionableMixerAudioSource(PositionableMixerAudioSourcePrivate &d, QObject *parent)
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
        AudioSource::close();
        if (d->start(bufferSize, sampleRate)) {
            d->setNextReadPositionToAll(d->position);
            return PositionableAudioSource::open(bufferSize, sampleRate);
        } else {
            return false;
        }
    }

    qint64 PositionableMixerAudioSource::processReading(const AudioSourceReadData &readData) {
        Q_D(PositionableMixerAudioSource);
        qint64 readLength;
        {
            auto channelCount = readData.buffer->channelCount();
            QMutexLocker locker(&d->mutex);
            auto bufferLength = length();
            if (d->tmpBuf.channelCount() < channelCount)
                d->tmpBuf.resize(channelCount);
            for (int i = 0; i < channelCount; i++) {
                readData.buffer->clear(i, readData.startPos, readData.length);
            }
            readLength = qBound(0ll, bufferLength - nextReadPosition(), readData.length);
            d->mix(readData, readLength);
            d->position += readLength;
        }
        if (!d->currentMagnitudes.empty())
                emit levelMetered(d->currentMagnitudes);
        return readLength;
    }

    /**
     * @copydoc MixerAudioSource::close()
     */
    void PositionableMixerAudioSource::close() {
        Q_D(PositionableMixerAudioSource);
        QMutexLocker locker(&d->mutex);
        d->stop();
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
                                  [](auto *src1, auto *src2) {
                                      return src1->length() < src2->length();
                                  }))
                ->length();
    }

    void PositionableMixerAudioSourcePrivate::setNextReadPositionToAll(qint64 pos) {
        std::for_each(sourceDict.constBegin(), sourceDict.constEnd(),
                      [=](const SourceInfo<PositionableAudioSource> &srcInfo) {
                          srcInfo.src->setNextReadPosition(pos);
                      });
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

    bool PositionableMixerAudioSource::addSource(PositionableAudioSource *src, bool takeOwnership) {
        if (src == this)
            return false;
        Q_D(PositionableMixerAudioSource);
        QMutexLocker locker(&d->mutex);
        src->setNextReadPosition(nextReadPosition());
        return d->addSource(src, takeOwnership, isOpen(), bufferSize(), sampleRate());
    }

    PositionableMixerAudioSource::SourceIterator
    PositionableMixerAudioSource::appendSource(PositionableAudioSource *src, bool takeOwnership) {
        Q_D(PositionableMixerAudioSource);
        QMutexLocker locker(&d->mutex);
        src->setNextReadPosition(nextReadPosition());
        return d->insertSource(d->sourceIteratorEnd(), src, takeOwnership, isOpen(), bufferSize(), sampleRate());
    }

    PositionableMixerAudioSource::SourceIterator
    PositionableMixerAudioSource::prependSource(PositionableAudioSource *src, bool takeOwnership) {
        Q_D(PositionableMixerAudioSource);
        QMutexLocker locker(&d->mutex);
        src->setNextReadPosition(nextReadPosition());
        return d->insertSource(d->firstSource(), src, takeOwnership, isOpen(), bufferSize(), sampleRate());
    }

    PositionableMixerAudioSource::SourceIterator
    PositionableMixerAudioSource::insertSource(const PositionableMixerAudioSource::SourceIterator &pos,
                                               PositionableAudioSource *src, bool takeOwnership) {
        Q_D(PositionableMixerAudioSource);
        QMutexLocker locker(&d->mutex);
        src->setNextReadPosition(nextReadPosition());
        return d->insertSource(pos, src, takeOwnership, isOpen(), bufferSize(), sampleRate());
    }

    bool PositionableMixerAudioSource::removeSource(PositionableAudioSource *src) {
        Q_D(PositionableMixerAudioSource);
        QMutexLocker locker(&d->mutex);
        return d->removeSource(src);
    }

    void PositionableMixerAudioSource::eraseSource(const PositionableMixerAudioSource::SourceIterator &srcIt) {
        Q_D(PositionableMixerAudioSource);
        QMutexLocker locker(&d->mutex);
        d->eraseSource(srcIt);
    }

    void PositionableMixerAudioSource::removeAllSources() {
        Q_D(PositionableMixerAudioSource);
        QMutexLocker locker(&d->mutex);
        d->removeAllSources();
    }

    void PositionableMixerAudioSource::moveSource(const PositionableMixerAudioSource::SourceIterator &pos, const SourceIterator &first, const SourceIterator &last) {
        Q_D(PositionableMixerAudioSource);
        QMutexLocker locker(&d->mutex);
        d->moveSource(pos, first, last);
    }

    void PositionableMixerAudioSource::swapSource(const PositionableMixerAudioSource::SourceIterator &first,
                                                  const PositionableMixerAudioSource::SourceIterator &second) {
        Q_D(PositionableMixerAudioSource);
        QMutexLocker locker(&d->mutex);
        d->swapSource(first, second);
    }

    QList<PositionableAudioSource *> PositionableMixerAudioSource::sources() const {
        Q_D(const PositionableMixerAudioSource);
        return d->sources();
    }

    PositionableMixerAudioSource::SourceIterator PositionableMixerAudioSource::firstSource() const {
        Q_D(const PositionableMixerAudioSource);
        return d->firstSource();
    }

    PositionableMixerAudioSource::SourceIterator PositionableMixerAudioSource::lastSource() const {
        Q_D(const PositionableMixerAudioSource);
        return d->lastSource();
    }

    PositionableMixerAudioSource::SourceIterator PositionableMixerAudioSource::sourceAt(int index) const {
        Q_D(const PositionableMixerAudioSource);
        return d->sourceAt(index);
    }

    PositionableMixerAudioSource::SourceIterator
    PositionableMixerAudioSource::findSource(PositionableAudioSource *src) const {
        Q_D(const PositionableMixerAudioSource);
        return d->findSource(src);
    }

    PositionableMixerAudioSource::SourceIterator PositionableMixerAudioSource::nullIterator() const {
        Q_D(const PositionableMixerAudioSource);
        return d->sourceIteratorEnd();
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

    bool PositionableMixerAudioSource::isMutedBySoloSetting(PositionableAudioSource *src) const {
        Q_D(const PositionableMixerAudioSource);
        return d->isMutedBySoloSetting(src);
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

    void PositionableMixerAudioSource::setLevelMeterChannelCount(int count) {
        Q_D(PositionableMixerAudioSource);
        QMutexLocker locker(&d->mutex);
        d->currentMagnitudes.resize(count);
    }

    int PositionableMixerAudioSource::levelMeterChannelCount() {
        Q_D(const PositionableMixerAudioSource);
        return d->currentMagnitudes.size();
    }

    /**
     * @fn void PositionableMixerAudioSource::levelMetered(const QVector<float> &values)
     * Emitted on each block processed. Outputs the magnitude of each channel.
     *
     * To use this signal, setLevelMeterChannelCount() must be set to non-zero.
     */

}
