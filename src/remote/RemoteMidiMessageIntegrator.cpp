/******************************************************************************
 * Copyright (c) 2024 CrSjimo                                                 *
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

#include "RemoteMidiMessageIntegrator.h"
#include "RemoteMidiMessageIntegrator_p.h"

#include <limits>

namespace talcs {
    RemoteMidiMessageIntegrator::RemoteMidiMessageIntegrator() : RemoteMidiMessageIntegrator(*new RemoteMidiMessageIntegratorPrivate) {

    }

    RemoteMidiMessageIntegrator::~RemoteMidiMessageIntegrator() {
        close();
    }

    quint64 RemoteMidiMessageIntegrator::sourceId() const {
        Q_D(const RemoteMidiMessageIntegrator);
        return d->sourceId;
    }

    bool RemoteMidiMessageIntegrator::open(qint64 bufferSize, double sampleRate) {
        Q_D(RemoteMidiMessageIntegrator);
        {
            QMutexLocker locker(&d->mutex);
            d->midiMessages.clear();
        }
        const bool opened = AbstractMidiMessageIntegrator::open(bufferSize, sampleRate);
        QMutexLocker locker(&d->mutex);
        d->messageSampleRate = opened ? sampleRate : 0;
        return opened;
    }

    void RemoteMidiMessageIntegrator::close() {
        Q_D(RemoteMidiMessageIntegrator);
        {
            QMutexLocker locker(&d->mutex);
            d->midiMessages.clear();
            d->messageSampleRate = 0;
        }
        AbstractMidiMessageIntegrator::close();
    }

    QList<IntegratedMidiMessage> RemoteMidiMessageIntegrator::fetch(qint64 length) {
        Q_D(RemoteMidiMessageIntegrator);
        QMutexLocker locker(&d->mutex);
        auto ret = d->midiMessages;
        d->midiMessages.clear();
        return ret;
    }

    void RemoteMidiMessageIntegrator::onThisBlockProcessInfo(const RemoteProcessInfo &processInfo) {
        Q_D(RemoteMidiMessageIntegrator);
        QMutexLocker locker(&d->mutex);
        d->midiMessages.clear();
        if (processInfo.containsInfo && d->messageSampleRate > 0) {
            const auto blockTime = MidiInputEvent::currentTime();
            auto midiMessagesRawData = &processInfo.midiMessages;
            d->midiMessages.reserve(midiMessagesRawData->size);
            auto midiMessagesRawDataPointer = reinterpret_cast<const char *>(midiMessagesRawData->messages);
            for (int i = 0; i < midiMessagesRawData->size; i++) {
                auto midiMessageRawData = reinterpret_cast<const RemoteMidiMessage *>(midiMessagesRawDataPointer);
                if (midiMessageRawData->size > 0 && midiMessageRawData->size <= std::numeric_limits<int>::max()) {
                    MidiMessage message(midiMessageRawData->data, static_cast<int>(midiMessageRawData->size),
                        blockTime + static_cast<double>(midiMessageRawData->position) / d->messageSampleRate);
                    if (message.isValid())
                        d->midiMessages.append({midiMessageRawData->position, message, d->sourceId});
                }
                midiMessagesRawDataPointer+= sizeof(RemoteMidiMessage) + midiMessageRawData->size - 1;
            }
        }
    }

    RemoteMidiMessageIntegrator::RemoteMidiMessageIntegrator(RemoteMidiMessageIntegratorPrivate &d) : AbstractMidiMessageIntegrator(d) {
    }
} // talcs
