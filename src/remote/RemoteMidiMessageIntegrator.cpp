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

namespace talcs {
    RemoteMidiMessageIntegrator::RemoteMidiMessageIntegrator() : RemoteMidiMessageIntegrator(*new RemoteMidiMessageIntegratorPrivate) {

    }

    RemoteMidiMessageIntegrator::~RemoteMidiMessageIntegrator() {

    }

    bool RemoteMidiMessageIntegrator::open(qint64 bufferSize, double sampleRate) {
        Q_D(RemoteMidiMessageIntegrator);
        QMutexLocker locker(&d->mutex);
        d->midiMessages.clear();
        return AbstractMidiMessageIntegrator::open(bufferSize, sampleRate);
    }

    void RemoteMidiMessageIntegrator::close() {
        Q_D(RemoteMidiMessageIntegrator);
        QMutexLocker locker(&d->mutex);
        d->midiMessages.clear();
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
        if (processInfo.containsInfo) {
            auto midiMessagesRawData = &processInfo.midiMessages;
            d->midiMessages.reserve(static_cast<int>(midiMessagesRawData->size));
            auto midiMessagesRawDataPointer = reinterpret_cast<const char *>(midiMessagesRawData->messages);
            for (int i = 0; i < midiMessagesRawData->size; i++) {
                auto midiMessageRawData = reinterpret_cast<const RemoteMidiMessage *>(midiMessagesRawDataPointer);
                d->midiMessages.append({
                    midiMessageRawData->position,
                    MidiMessage(midiMessageRawData->data, static_cast<int>(midiMessageRawData->size),
                                static_cast<double>(midiMessageRawData->position))
                });
                midiMessagesRawDataPointer+= sizeof(RemoteMidiMessage) + midiMessageRawData->size - 1;
            }
        }
    }

    RemoteMidiMessageIntegrator::RemoteMidiMessageIntegrator(RemoteMidiMessageIntegratorPrivate &d) : AbstractMidiMessageIntegrator(d) {
    }
} // talcs