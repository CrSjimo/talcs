/******************************************************************************
 * Copyright (c) 2023-2024 CrSjimo                                            *
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

#include "AudioMidiStreamMixer.h"
#include "AudioMidiStreamMixer_p.h"

#include <algorithm>
#include <cmath>

namespace talcs {

    AudioMidiStreamMixer::AudioMidiStreamMixer() : AudioMidiStream(*new AudioMidiStreamMixerPrivate) {
    }

    AudioMidiStreamMixer::~AudioMidiStreamMixer() {
        close();
    }

    bool AudioMidiStreamMixer::open(qint64 bufferSize, double sampleRate) {
        Q_D(AudioMidiStreamMixer);
        QMutexLocker locker(&d->mutex);
        AudioMidiStream::close();
        for (auto stream : d->streams)
            stream->close();
        if (bufferSize <= 0 || !std::isfinite(sampleRate) || sampleRate <= 0)
            return false;
        for (auto stream : d->streams) {
            if (!stream->open(bufferSize, sampleRate)) {
                for (auto opened : d->streams)
                    opened->close();
                return false;
            }
        }
        if (AudioMidiStream::open(bufferSize, sampleRate))
            return true;
        for (auto stream : d->streams)
            stream->close();
        return false;
    }

    void AudioMidiStreamMixer::close() {
        Q_D(AudioMidiStreamMixer);
        QMutexLocker locker(&d->mutex);
        for (auto stream : d->streams)
            stream->close();
        AudioMidiStream::close();
    }

    bool AudioMidiStreamMixer::addStream(AudioMidiStream *stream) {
        Q_D(AudioMidiStreamMixer);
        QMutexLocker locker(&d->mutex);
        if (!stream || stream == this || d->streams.contains(stream))
            return false;
        if (isOpen() && !stream->open(bufferSize(), sampleRate())) {
            stream->close();
            return false;
        }
        d->streams.append(stream);
        return true;
    }

    void AudioMidiStreamMixer::removeStream(AudioMidiStream *stream) {
        Q_D(AudioMidiStreamMixer);
        QMutexLocker locker(&d->mutex);
        if (d->streams.removeOne(stream))
            stream->close();
    }

    qint64 AudioMidiStreamMixer::processReading(const AudioSourceReadData &readData, const QList<IntegratedMidiMessage> &events) {
        Q_D(AudioMidiStreamMixer);
        QMutexLocker locker(&d->mutex);
        const auto channels = readData.buffer->channelCount();
        d->scratch.resize(channels, readData.length);
        for (int channel = 0; channel < channels; ++channel)
            readData.buffer->clear(channel, readData.startPos, readData.length);
        for (auto stream : d->streams) {
            d->scratch.clear();
            const auto length = std::clamp(stream->read({&d->scratch, 0, readData.length, readData.silentFlags}, events), qint64{0}, readData.length);
            for (int channel = 0; channel < channels; ++channel)
                readData.buffer->addSampleRange(channel, readData.startPos, length, d->scratch, channel, 0);
        }
        return readData.length;
    }

}
