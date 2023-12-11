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

#include "AudioFormatInputSource.h"
#include "AudioFormatInputSource_p.h"

#include <cmath>

#include <QDebug>

#include <TalcsCore/InterleavedAudioDataWrapper.h>
#include <TalcsFormat/AudioFormatIO.h>

namespace talcs {

    static inline qint64 outPositionToIn(qint64 position, double ratio) {
        return qRound64(position / ratio);
    }

    static inline qint64 inPositionToOut(qint64 position, double ratio) {
        return qRound64(position * ratio);
    }

    /**
     * @class AudioFormatInputSource
     * @brief Takes audio from an AudioFormatIO object, resamples it to a specified sample rate, and produces it out.
     *
     * r8brain is used to resample the audio.
     * @see @link URL https://github.com/avaneev/r8brain-free-src @endlink
     */

    /**
     * Constructor.
     */
    AudioFormatInputSource::AudioFormatInputSource(AudioFormatIO *audioFormatIo, bool takeOwnership)
            : AudioFormatInputSource(*new AudioFormatInputSourcePrivate) {
        setAudioFormatIo(audioFormatIo, takeOwnership);
    }

    AudioFormatInputSource::~AudioFormatInputSource() {
        Q_D(AudioFormatInputSource);
        if (d->takeOwnership) {
            delete d->io;
        }
    }

    AudioFormatInputSource::AudioFormatInputSource(AudioFormatInputSourcePrivate &d) : PositionableAudioSource(d) {
    }

    AudioFormatInputSourcePrivate::AudioFormatInputResampler::AudioFormatInputResampler(double ratio, qint64 bufferSize,
                                                                                        int channelCount,
                                                                                        AudioFormatInputSourcePrivate *d)
            : R8BrainMultichannelResampler(ratio, bufferSize, channelCount), d(d) {
    }

    void AudioFormatInputSourcePrivate::AudioFormatInputResampler::read(const AudioSourceReadData &readData) {
        d->io->seek(d->inPosition);
        tmpBuf.resize(readData.length * channelCount());
        auto inLength = d->io->read(tmpBuf.data(), readData.length);
        InterleavedAudioDataWrapper wrapper(tmpBuf.data(), channelCount(), readData.length);
        for (int i = 0; i < channelCount(); i++) {
            readData.buffer->setSampleRange(i, readData.startPos, inLength, wrapper, i, 0);
            readData.buffer->clear(i, readData.startPos + inLength, readData.length - inLength);
        }
        d->inPosition += inLength;
    }

    qint64 AudioFormatInputSource::read(const AudioSourceReadData &readData) {
        Q_D(AudioFormatInputSource);
        QMutexLocker locker(&d->mutex);
        Q_ASSERT(d->io && isOpen());
        auto readLength = qMin(readData.length, length() - d->position);
        d->resampler->process(readData);
        if (d->doStereoize && d->io->channelCount() == 1 && readData.buffer->channelCount() > 1) {
            readData.buffer->setSampleRange(1, readData.startPos, readLength, *readData.buffer, 0, readData.startPos);
        }
        d->position += readLength;
        return readLength;
    }

    qint64 AudioFormatInputSource::length() const {
        Q_D(const AudioFormatInputSource);
        if (!d->io || !isOpen())
            return 0;
        return inPositionToOut(d->io->length(), d->ratio);
    }

    void AudioFormatInputSource::setNextReadPosition(qint64 pos) {
        Q_D(AudioFormatInputSource);
        QMutexLocker locker(&d->mutex);
        if (pos != d->position) {
            if (d->resampler)
                d->resampler->reset();
            if (d->io && d->io->openMode())
                d->io->seek(outPositionToIn(pos, d->ratio));
            d->inPosition = outPositionToIn(pos, d->ratio);
        }
        PositionableAudioSource::setNextReadPosition(pos);
    }

    bool AudioFormatInputSource::open(qint64 bufferSize, double sampleRate) {
        Q_D(AudioFormatInputSource);
        QMutexLocker locker(&d->mutex);
        if (!d->io)
            return false;
        if (d->io->open(QIODevice::ReadOnly)) {
            d->ratio = sampleRate / d->io->sampleRate();
            d->resampler = new AudioFormatInputSourcePrivate::AudioFormatInputResampler(d->ratio, bufferSize,
                                                                                        d->io->channelCount(), d);
            d->io->seek(outPositionToIn(d->position, d->ratio));
            return AudioStreamBase::open(bufferSize, sampleRate);
        } else
            return false;
    }

    void AudioFormatInputSource::close() {
        Q_D(AudioFormatInputSource);
        QMutexLocker locker(&d->mutex);
        if (!d->io)
            return;
        d->io->close();
        d->ratio = 0;
        delete d->resampler;
        d->resampler = nullptr;
    }

    /**
     * Resets the internal AudioFormatIO object to another one.
     *
     * Note that this function should not be called when the source is open.
     */
    void AudioFormatInputSource::setAudioFormatIo(AudioFormatIO *audioFormatIo, bool takeOwnership) {
        Q_ASSERT(!isOpen());
        if (isOpen()) {
            qWarning() << "Cannot set audio format io when source is opened.";
            return;
        }
        Q_D(AudioFormatInputSource);
        QMutexLocker locker(&d->mutex);
        d->io = audioFormatIo;
        d->takeOwnership = takeOwnership;
        if (d->io && d->io->openMode())
            d->io->seek(d->inPosition);
        if (d->resampler)
            d->resampler->reset();
    }

    /**
     * Gets the AudioFormatIO that is currently being used.
     */
    AudioFormatIO *AudioFormatInputSource::audioFormatIo() const {
        Q_D(const AudioFormatInputSource);
        return d->io;
    }

    /**
     * Clears the internal buffer of the resampler.
     *
     * @note The resampler will be flushed when the read position is changed.
     */
    void AudioFormatInputSource::flush() {
        Q_D(AudioFormatInputSource);
        QMutexLocker locker(&d->mutex);
        if (d->resampler)
            d->resampler->reset();
    }

    /**
     * Sets whether this source produces stereo audio when the AudioFormatIO outputs mono.
     */
    void AudioFormatInputSource::setStereoize(bool stereoize) {
        Q_D(AudioFormatInputSource);
        QMutexLocker locker(&d->mutex);
        d->doStereoize = stereoize;
    }

    /**
     * Gets whether this source produces stereo audio when the AudioFormatIO outputs mono.
     */
    bool AudioFormatInputSource::stereoize() const {
        Q_D(const AudioFormatInputSource);
        return d->doStereoize;
    }

    bool AudioFormatInputSource::isDuplicatable() const {
        Q_D(const AudioFormatInputSource);
        return !d->io || d->io->isDuplicatable();
    }

    DuplicatableObject *AudioFormatInputSource::duplicate() const {
        Q_D(const AudioFormatInputSource);
        if (!d->io)
            return new AudioFormatInputSource;
        if (d->io->isDuplicatable())
            return new AudioFormatInputSource(static_cast<AudioFormatIO *>(d->io->duplicate()), d->takeOwnership);
        return nullptr;
    }

}