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

#include "AudioSourceProcessorBase.h"
#include "AudioSourceProcessorBase_p.h"

#include "source/AudioSource.h"

namespace talcs {

    /**
     * @class AudioSourceProcessorBase
     * @brief Base class for objects that process the audio produced by an AudioSource object.
     */

    /**
     * Constructor.
     */
    AudioSourceProcessorBase::AudioSourceProcessorBase(AudioSource *src, qint64 length, QObject *parent)
        : AudioSourceProcessorBase(*new AudioSourceProcessorBasePrivate, src, length, parent) {
    }

    AudioSourceProcessorBase::AudioSourceProcessorBase(AudioSourceProcessorBasePrivate &d, AudioSource *src,
                                                       qint64 length, QObject *parent)
        : d_ptr(&d) {
        d.q_ptr = this;
        d.src = src;
        d.length = length;
    }

    AudioSourceProcessorBase::~AudioSourceProcessorBase() = default;

    /**
     * @enum Status
     * The possible statuses.
     * @var AudioSourceProcessorBase::Ready
     * The processor is ready to start.
     *
     * @var AudioSourceProcessorBase::Processing
     * The processor is processing the audio now.
     *
     * @var AudioSourceProcessorBase::Failed
     * The processor occurs an error while processing.
     *
     * @var AudioSourceProcessorBase::Interrupted
     * The processor is interrupted manually.
     *
     * @var AudioSourceProcessorBase::Completed
     * The processor finishes processing the audio successfully.
     */

    /**
     * Gets the status of this processor.
     */
    AudioSourceProcessorBase::Status AudioSourceProcessorBase::status() const {
        Q_D(const AudioSourceProcessorBase);
        return d->status;
    }

    /**
     * Gets the AudioSource object that this processor uses.
     */
    AudioSource *AudioSourceProcessorBase::source() const {
        Q_D(const AudioSourceProcessorBase);
        return d->src;
    }

    /**
     * Gets the length of audio to process.
     */
    qint64 AudioSourceProcessorBase::length() const {
        Q_D(const AudioSourceProcessorBase);
        return d->length;
    }

    /**
     * Interrupts the process. This function is thread-safe.
     *
     * Note that do not use queued connection to connect this function to any signal, otherwise this function will not
     * be invoked before completed, since the thread is blocked while writing.
     */
    void AudioSourceProcessorBase::interrupt() {
        Q_D(AudioSourceProcessorBase);
        d->stopRequested = true;
    }

    /**
     * Starts processing.
     *
     * Note that in most cases, this function should be called from another thread to avoid from blocking the main thread. It is
     * recommended to move this object to another thread using
     * [QObject::moveToThread](https://doc.qt.io/qt-5/qobject.html#moveToThread)() and connect signals and slots
     * across different threads.
     * @see [QThread](https://doc.qt.io/qt-5/qthread.html)
     */
    void AudioSourceProcessorBase::start() {
        Q_D(AudioSourceProcessorBase);
        d->status = Processing;
        auto buf = prepareBuffer();
        qint64 framesToRead = d->length;
        qint64 blockSize = d->src->bufferSize();
        while (framesToRead > 0) {
            if (d->stopRequested) {
                d->status = Interrupted;
                break;
            }
            qint64 readLength = std::min(framesToRead, blockSize);
            qint64 framesRead = d->src->read({buf, 0, readLength});
            if (framesRead == 0) {
                d->status = Failed;
                break;
            }
            if (!processBlock(d->length - framesToRead, framesRead)) {
                d->status = Failed;
                break;
            }
            framesToRead -= framesRead;
            emit blockProcessed(d->length - framesToRead);
        }
        if (d->status == Processing) {
            processWillFinish();
            d->status = Completed;
        } else {
            processWillFinish();
        }
        emit finished();
    }

    /**
     * @fn void AudioSourceProcessorBase::blockProcessed(qint64 processedSampleCount)
     * Emitted when each block has been processed.
     */

    /**
     * @fn void AudioSourceProcessorBase::finished()
     * Emitted when processing is finished.
     */

    /**
     * @fn IAudioSampleContainer *AudioSourceProcessorBase::prepareBuffer()
     * Derived classes should implement this function to create the buffer for reading from the source.
     *
     * The size of the buffer prepared should be identical to the buffer size of the AudioSource object to read.
     */

    /**
     * @fn bool AudioSourceProcessorBase::processBlock(qint64 processedSampleCount, qint64 samplesToProcess)
     * Derived classes should implement this function to process each block.
     *
     * When this function is called, the block read from the source has been stored in the buffer that prepareBuffer() returns.
     * @returns @c true if successful
     * @see prepareBuffer()
     */

    /**
     * @fn void AudioSourceProcessorBase::processWillFinish()
     * Derived classes should implement this function to do some clean-ups after processing.
     *
     * Note that the processing might be either successful, interrupted manually, or occurs an error.
     * @see status()
     */

}