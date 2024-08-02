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

#include "AudioSource.h"
#include "AudioSource_p.h"

namespace talcs {

    /**
     * @struct AudioSourceReadData
     * @brief The object that contains the target for AudioSource to fill data in
     *
     * Note that the "read" in the class name is a past participle, which means the data is "read" from an AudioSource
     * object.
     * @see AudioSource, IAudioSampleContainer
     * @var AudioSourceReadData::buffer
     * The IAudioSampleContainer object used
     *
     * @var AudioSourceReadData::startPos
     * The start position of the specified range
     *
     * @var AudioSourceReadData::length
     * The length of the specified range
     *
     * @var AudioSourceReadData::silentFlags
     * Bitwise flags of whether a specified channel is silent
     */

    /**
     * Implicit constructor. The data is filled from the start of the IAudioSampleContainer object and the length read
     * is to be the number of samples of it.
     */
    AudioSourceReadData::AudioSourceReadData(IAudioSampleContainer *buffer)
        : AudioSourceReadData(buffer, 0, buffer->sampleCount()) {
    }

    /**
     * The data is filled into a specified range in the IAudioSampleContainer object.
     */
    AudioSourceReadData::AudioSourceReadData(IAudioSampleContainer *buffer, qint64 startPos, qint64 length, int silentFlags)
        : buffer(buffer), startPos(startPos), length(length), silentFlags(silentFlags) {
    }

    /**
     * @class AudioSource
     * @brief Base class for sources from which audio data can be streamly read
     *
     * An AudioSource object can be filtered. In this case, the audio data produced by processReading() will be sent to
     * the filter's read() function.
     */

    /**
     * Default constructor.
     */
    AudioSource::AudioSource() : AudioSource(*new AudioSourcePrivate) {
    }

    AudioSource::AudioSource(AudioSourcePrivate &d) : d_ptr(&d) {
        d.q_ptr = this;
    }
    AudioSource::~AudioSource() = default;

    bool AudioSource::open(qint64 bufferSize, double sampleRate) {
        Q_D(AudioSource);
        QMutexLocker locker(&d->filterMutex);
        if (d->filter.loadRelaxed() && !d->filter.loadRelaxed()->open(bufferSize, sampleRate)) {
            return false;
        }
        return AudioStreamBase::open(bufferSize, sampleRate);
    }

    void AudioSource::close() {
        Q_D(AudioSource);
        QMutexLocker locker(&d->filterMutex);
        if (d->filter.loadRelaxed())
            d->filter.loadRelaxed()->close();
        AudioStreamBase::close();
    }

    /**
     * @fn qint64 AudioSource::read(const AudioSourceReadData &readData)
     * Reads audio data from the source.
     *
     * See @ref doc/reading_from_a_source.md "Reading from a source" for detailed descriptions.
     *
     * @param readData see docs in class AudioSourceReadData
     * @returns the actual length of audio read measured in samples
     */

    void AudioSource::applyFilterImpl(const AudioSourceReadData &readData, qint64 l) {
        Q_D(AudioSource);
        if (!d->filter) return;
        QMutexLocker locker(&d->filterMutex);
        if (d->filter.loadRelaxed()) {
            d->filter.loadRelaxed()->read({readData.buffer, readData.startPos, l, readData.silentFlags});
        }
    }

    /**
     * Sets the reading filter.
     */
    void AudioSource::setReadingFilter(AudioSource *filter) {
        Q_D(AudioSource);
        QMutexLocker locker(&d->filterMutex);
        d->filter = filter;
        if (isOpen())
            filter->open(bufferSize(), sampleRate());
    }

    /**
     * Gets the reading filter.
     */
    AudioSource *AudioSource::readingFilter() const {
        Q_D(const AudioSource);
        return d->filter;
    }

    /**
     * @fn qint64 AudioSource::processReading(const AudioSourceReadData &readData)
     * Derived classes override this function to produce audio.
     */

}
