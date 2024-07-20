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

#include "AudioSourceWriter.h"
#include "AudioSourceWriter_p.h"

#include <QMutex>

#include <TalcsCore/AudioSource.h>
#include <TalcsCore/InterleavedAudioDataWrapper.h>

#include <TalcsFormat/AudioFormatIO.h>

namespace talcs {

    /**
     * @class AudioSourceWriter
     * @brief The AudioSourceWriter class provides interfaces to write audio.
     *
     * The audio is read from a AudioSource object, and write to an AudioFormatIO object.
     * @see AudioSource, AudioFormatIO
     */


    /**
     * Constructor.
     *
     * AudioSource object and the AudioFormatIO object must be opened before writing is started, and closed
     * manually after writing.
     *
     * Note that this object does not take the ownership of both objects.
     */
    AudioSourceWriter::AudioSourceWriter(AudioSource *src, AudioFormatIO *outFile, qint64 length, QObject *parent)
        : AudioSourceProcessorBase(*new AudioSourceWriterPrivate, src, length, parent) {
        Q_D(AudioSourceWriter);
        d->outFile = outFile;
    }

    /**
     * Destructor.
     */
    AudioSourceWriter::~AudioSourceWriter() = default;

    IAudioSampleContainer *AudioSourceWriter::prepareBuffer() {
        Q_D(AudioSourceWriter);
        auto p = new float[d->src->bufferSize() * d->outFile->channelCount()];
        d->buf = new InterleavedAudioDataWrapper(p, d->outFile->channelCount(), d->src->bufferSize());
        return d->buf;
    }

    bool AudioSourceWriter::processBlock(qint64 processedSampleCount, qint64 samplesToProcess) {
        Q_D(AudioSourceWriter);
        d->outFile->write(d->buf->data(), samplesToProcess);
        return true;
    }

    void AudioSourceWriter::processWillFinish() {
        Q_D(AudioSourceWriter);
        delete d->buf->data();
        delete d->buf;
        d->buf = nullptr;
    }

}