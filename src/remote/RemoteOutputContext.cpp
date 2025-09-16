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

#include "RemoteOutputContext.h"
#include "RemoteOutputContext_p.h"

#include <QThread>

#include <TalcsCore/MixerAudioSource.h>

#include <TalcsDevice/AudioSourcePlayback.h>

#include <TalcsRemote/RemoteSocket.h>
#include <TalcsRemote/RemoteAudioDevice.h>

namespace talcs {
    RemoteOutputContext::RemoteOutputContext(QObject *parent) : AbstractOutputContext(parent), d_ptr(new RemoteOutputContextPrivate) {
        Q_D(RemoteOutputContext);
        d->q_ptr = this;
    }

    RemoteOutputContext::~RemoteOutputContext() = default;

    bool RemoteOutputContext::initialize(quint16 serverPort, quint16 clientPort) {
        Q_D(RemoteOutputContext);
        auto socket = std::make_unique<RemoteSocket>(serverPort, clientPort);
        if (!socket->startServer(QThread::idealThreadCount()))
            return false;
        d->device = std::make_unique<RemoteAudioDevice>(socket.get(), "TALCS Virtual Remote Audio");
        connect(d->device.get(), &RemoteAudioDevice::remoteOpened, this, [d](qint64 bufferSize, double sampleRate, int maxChannelCount) {
            d->handleRemoteDeviceRemoteOpened(bufferSize, sampleRate, maxChannelCount);
        });
        setDevice(d->device.get());
        d->socket = std::move(socket);
        return true;
    }

    bool RemoteOutputContext::establishConnection() {
        Q_D(RemoteOutputContext);
        return d->socket->startClient();
    }

    talcs::RemoteSocket *RemoteOutputContext::socket() const {
        Q_D(const RemoteOutputContext);
        return d->socket.get();
    }

    talcs::RemoteAudioDevice *RemoteOutputContext::remoteAudioDevice() const {
        Q_D(const RemoteOutputContext);
        return d->device.get();
    }

    void RemoteOutputContextPrivate::handleRemoteDeviceRemoteOpened(qint64 bufferSize, double sampleRate, int maxChannelCount) {
        Q_Q(RemoteOutputContext);
        auto oldBufferSize = device->bufferSize();
        auto oldSampleRate = device->sampleRate();
        device->open(bufferSize, sampleRate);
        if (bufferSize != oldBufferSize) {
            emit q->bufferSizeChanged(bufferSize);
        }
        if (!qFuzzyCompare(sampleRate, oldSampleRate)) {
            emit q->sampleRateChanged(sampleRate);
        }
        q->controlMixer()->open(bufferSize, sampleRate);
        device->start(q->playback());
        emit q->deviceChanged();
    }
}
