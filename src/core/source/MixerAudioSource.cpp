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

#include "MixerAudioSource.h"
#include "MixerAudioSource_p.h"

namespace talcs {

    /**
     * @class MixerAudioSource
     * @brief The object that mixes the output of other AudioSource objects.
     *
     * This is one of the @ref doc/object_binding.md "object-binding" classes.
     *
     * @see PositionableMixerAudioSource
     */

    /**
     * Default constructor.
     */
    MixerAudioSource::MixerAudioSource(QObject *parent) : MixerAudioSource(*new MixerAudioSourcePrivate, parent) {
        qRegisterMetaType<QVector<float>>();
    }

    MixerAudioSource::MixerAudioSource(MixerAudioSourcePrivate &d, QObject *parent) : QObject(parent), AudioSource(d) {
    }

    /**
     * Destructor.
     *
     * If the object is not close, it will be closed now.
     */
    MixerAudioSource::~MixerAudioSource() {
        Q_D(MixerAudioSource);
        MixerAudioSource::close();
        d->deleteOwnedSources();
    }

    /**
     * @copydoc AudioSource::open()
     *
     * The function also opens all input sources.
     */
    bool MixerAudioSource::open(qint64 bufferSize, double sampleRate) {
        Q_D(MixerAudioSource);
        QMutexLocker locker(&d->mutex);
        if (d->start(bufferSize, sampleRate)) {
            return AudioSource::open(bufferSize, sampleRate);
        } else {
            return false;
        }
    }

    qint64 MixerAudioSource::processReading(const AudioSourceReadData &readData) {
        Q_D(MixerAudioSource);
        qint64 readLength = readData.length;
        auto channelCount = readData.buffer->channelCount();
        {
            QMutexLocker locker(&d->mutex);
            if (d->tmpBuf.channelCount() < channelCount)
                d->tmpBuf.resize(channelCount);
            for (int i = 0; i < channelCount; i++) {
                readData.buffer->clear(i, readData.startPos, readLength);
            }
            readLength = d->mix(readData, readLength);
        }

        if (!d->currentMagnitudes.empty())
                emit levelMetered(d->currentMagnitudes);
        return readLength;
    }

    /**
     * @copydoc AudioSource::close()
     *
     * The function also closes all input sources.
     */
    void MixerAudioSource::close() {
        Q_D(MixerAudioSource);
        QMutexLocker locker(&d->mutex);
        d->stop();
        AudioSource::close();
    }

    bool MixerAudioSource::addSource(AudioSource *src, bool takeOwnership) {
        if (src == this)
            return false;
        Q_D(MixerAudioSource);
        QMutexLocker locker(&d->mutex);
        return d->addSource(src, takeOwnership, isOpen(), bufferSize(), sampleRate());
    }

    MixerAudioSource::SourceIterator MixerAudioSource::appendSource(AudioSource *src, bool takeOwnership) {
        Q_D(MixerAudioSource);
        QMutexLocker locker(&d->mutex);
        return d->insertSource(d->sourceIteratorEnd(), src, takeOwnership, isOpen(), bufferSize(), sampleRate());
    }

    MixerAudioSource::SourceIterator MixerAudioSource::prependSource(AudioSource *src, bool takeOwnership) {
        Q_D(MixerAudioSource);
        QMutexLocker locker(&d->mutex);
        return d->insertSource(d->firstSource(), src, takeOwnership, isOpen(), bufferSize(), sampleRate());
    }

    MixerAudioSource::SourceIterator
    MixerAudioSource::insertSource(const MixerAudioSource::SourceIterator &pos, AudioSource *src,
                                   bool takeOwnership) {
        Q_D(MixerAudioSource);
        QMutexLocker locker(&d->mutex);
        return d->insertSource(pos, src, takeOwnership, isOpen(), bufferSize(), sampleRate());
    }

    bool MixerAudioSource::removeSource(AudioSource *src) {
        Q_D(MixerAudioSource);
        QMutexLocker locker(&d->mutex);
        return d->removeSource(src);
    }

    void MixerAudioSource::eraseSource(const MixerAudioSource::SourceIterator &srcIt) {
        Q_D(MixerAudioSource);
        QMutexLocker locker(&d->mutex);
        d->eraseSource(srcIt);
    }

    void MixerAudioSource::removeAllSources() {
        Q_D(MixerAudioSource);
        QMutexLocker locker(&d->mutex);
        d->removeAllSources();
    }

    void MixerAudioSource::moveSource(const MixerAudioSource::SourceIterator &pos,
                                      const MixerAudioSource::SourceIterator &target) {
        Q_D(MixerAudioSource);
        QMutexLocker locker(&d->mutex);
        d->moveSource(pos, target);
    }

    void MixerAudioSource::swapSource(const MixerAudioSource::SourceIterator &first,
                                      const MixerAudioSource::SourceIterator &second) {
        Q_D(MixerAudioSource);
        QMutexLocker locker(&d->mutex);
        d->swapSource(first, second);
    }

    QList<AudioSource *> MixerAudioSource::sources() const {
        Q_D(const MixerAudioSource);
        return d->sources();
    }

    MixerAudioSource::SourceIterator MixerAudioSource::firstSource() const {
        Q_D(const MixerAudioSource);
        return d->firstSource();
    }

    MixerAudioSource::SourceIterator MixerAudioSource::lastSource() const {
        Q_D(const MixerAudioSource);
        return d->lastSource();
    }

    MixerAudioSource::SourceIterator MixerAudioSource::findSource(AudioSource *src) const {
        Q_D(const MixerAudioSource);
        return d->findSource(src);
    }

    MixerAudioSource::SourceIterator MixerAudioSource::nullIterator() const {
        Q_D(const MixerAudioSource);
        return d->sourceIteratorEnd();
    }

    void MixerAudioSource::setSourceSolo(AudioSource *src, bool isSolo) {
        Q_D(MixerAudioSource);
        QMutexLocker locker(&d->mutex);
        d->setSourceSolo(src, isSolo);
    }

    bool MixerAudioSource::isSourceSolo(AudioSource *src) const {
        Q_D(const MixerAudioSource);
        return d->isSourceSolo(src);
    }

    bool MixerAudioSource::isMutedBySoloSetting(AudioSource *src) const {
        Q_D(const MixerAudioSource);
        return d->isMutedBySoloSetting(src);
    }

    void MixerAudioSource::setGain(float gain) {
        Q_D(MixerAudioSource);
        QMutexLocker locker(&d->mutex);
        d->gain = gain;
    }

    float MixerAudioSource::gain() const {
        Q_D(const MixerAudioSource);
        return d->gain;
    }

    void MixerAudioSource::setPan(float pan) {
        Q_D(MixerAudioSource);
        QMutexLocker locker(&d->mutex);
        d->pan = pan;
    }

    float MixerAudioSource::pan() const {
        Q_D(const MixerAudioSource);
        return d->pan;
    }

    void MixerAudioSource::setRouteChannels(bool routeChannels) {
        Q_D(MixerAudioSource);
        QMutexLocker locker(&d->mutex);
        d->routeChannels = routeChannels;
    }

    bool MixerAudioSource::routeChannels() const {
        Q_D(const MixerAudioSource);
        return d->routeChannels;
    }

    void MixerAudioSource::setSilentFlags(int silentFlags) {
        Q_D(MixerAudioSource);
        QMutexLocker locker(&d->mutex);
        d->silentFlags = silentFlags;
    }

    int MixerAudioSource::silentFlags() const {
        Q_D(const MixerAudioSource);
        return d->silentFlags;
    }

    void MixerAudioSource::setLevelMeterChannelCount(int count) {
        Q_D(MixerAudioSource);
        QMutexLocker locker(&d->mutex);
        d->currentMagnitudes.resize(count);
    }

    int MixerAudioSource::levelMeterChannelCount() {
        Q_D(const MixerAudioSource);
        return d->currentMagnitudes.size();
    }

    /**
     * @fn void MixerAudioSource::levelMetered(const QVector<float> &values)
     * Emitted on each block processed. Outputs the magnitude of each channel.
     *
     * To use this signal, setLevelMeterChannelCount() must be set to non-zero.
     */

}