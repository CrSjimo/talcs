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

#include "MidiInputDevice.h"
#include "MidiInputDevice_p.h"

#include <QDebug>

#include "MidiMessageListener.h"

namespace talcs {
    MidiInputDevice::MidiInputDevice(int deviceIndex, QObject *parent) : QObject(parent), d_ptr(new MidiInputDevicePrivate) {
        Q_D(MidiInputDevice);
        d->q_ptr = this;
        d->portNumber = deviceIndex;
        try {
            d->midi.reset(new RtMidiIn);
            setName(QString::fromStdString(d->midi->getPortName(deviceIndex)));
        } catch (RtMidiError &e) {
            setErrorString(QString::fromStdString(e.getMessage()));
        }
    }

    MidiInputDevice::~MidiInputDevice() {
        close();
    }

    void MidiInputDevicePrivate::rtmidiCallback(double timeStamp, std::vector<unsigned char> *message, void *userData) {
        auto d = reinterpret_cast<MidiInputDevicePrivate *>(userData);
        MidiMessage msg(message->data(), static_cast<int>(message->size()), timeStamp);
        d->listener.messageCallback(msg);
    }

    void MidiInputDevicePrivate::rtmidiErrorCallback(RtMidiError::Type type, const std::string &errorText, void *userData) {
        Q_UNUSED(type)
        auto d = reinterpret_cast<MidiInputDevicePrivate *>(userData);
        d->q_ptr->setErrorString(QString::fromStdString(errorText));
        d->listener.errorCallback(QString::fromStdString(errorText));
    }

    int MidiInputDevice::deviceIndex() const {
        Q_D(const MidiInputDevice);
        return d->portNumber;
    }

    bool MidiInputDevice::open() {
        Q_D(MidiInputDevice);
        if (!d->midi)
            return false;
        try {
            d->midi->openPort(d->portNumber);
            d->midi->ignoreTypes(false, false, false);
            d->listener.deviceWillStartCallback(this);
            d->midi->setCallback(&MidiInputDevicePrivate::rtmidiCallback, d);
            d->midi->setErrorCallback(&MidiInputDevicePrivate::rtmidiErrorCallback, d);
        } catch (RtMidiError &e) {
            setErrorString(QString::fromStdString(e.getMessage()));
            return false;
        }
        setErrorString({});
        return true;
    }

    bool MidiInputDevice::isOpen() const {
        Q_D(const MidiInputDevice);
        return d->midi && d->midi->isPortOpen();
    }

    void MidiInputDevice::close() {
        Q_D(MidiInputDevice);
        if (d->midi)
            d->midi->closePort();
        d->listener.deviceStoppedCallback();
        setErrorString({});
    }

    MidiMessageListener *MidiInputDevice::listener() const {
        Q_D(const MidiInputDevice);
        return &d->listener;
    }

    QStringList MidiInputDevice::devices() {
        QStringList a;
        try {
            RtMidiIn tmp;
            int count = static_cast<int>(tmp.getPortCount());
            a.reserve(count);
            for (int i = 0; i < count; i++) {
                a.append(QString::fromStdString(tmp.getPortName(i)));
            }
        } catch (RtMidiError &e) {
            qWarning() << "MidiInputDevice:" << QString::fromStdString(e.getMessage());
        }
        return a;
    }
} // talcs