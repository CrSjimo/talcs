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

#include "MidiInputDevice.h"
#include "MidiInputDevice_p.h"

#include <QDebug>

#include <rtmidi/RtMidi.h>

#include "MidiInputDeviceCallback.h"

namespace talcs {
    MidiInputDevice::MidiInputDevice(int deviceIndex, QObject *parent) : QObject(parent), d(new MidiInputDevicePrivate) {
        d->portNumber = deviceIndex;
        try {
            d->midi.reset(new RtMidiIn);
            setName(d->midi->getPortName(deviceIndex).c_str());
        } catch (RtMidiError &e) {
            setErrorString(e.getMessage().c_str());
        }
    }

    MidiInputDevice::~MidiInputDevice() = default;

    static void rtmidiCallback(double timeStamp, std::vector<unsigned char> *message, void *userData) {
        auto d = reinterpret_cast<MidiInputDevicePrivate *>(userData);
        MidiMessage msg(message->data(), message->size(), timeStamp);
        std::for_each(d->listeners.begin(), d->listeners.end(), [&](MidiInputDeviceCallback *cb) {
            cb->workCallback(msg);
        });
    }

    static void rtmidiErrorCallback(RtMidiError::Type type, const std::string &errorText, void *userData) {
        Q_UNUSED(type);
        auto d = reinterpret_cast<MidiInputDevicePrivate *>(userData);
        std::for_each(d->listeners.begin(), d->listeners.end(), [&](MidiInputDeviceCallback *cb) {
            cb->errorCallback(errorText.c_str());
        });
    }

    bool MidiInputDevice::open() {
        if (!d->midi)
            return false;
        try {
            d->midi->openPort(d->portNumber);
            d->midi->ignoreTypes(false, false, false);
            std::for_each(d->listeners.begin(), d->listeners.end(), [&](MidiInputDeviceCallback *cb) {
                cb->deviceWillStartCallback(this);
            });
            d->midi->setCallback(&rtmidiCallback, d.data());
            d->midi->setErrorCallback(&rtmidiErrorCallback, d.data());
        } catch (RtMidiError &e) {
            setErrorString(e.getMessage().c_str());
            return false;
        }
        setErrorString({});
        return true;
    }

    bool MidiInputDevice::isOpen() const {
        return d->midi && d->midi->isPortOpen();
    }

    void MidiInputDevice::close() {
        if (d->midi)
            d->midi->closePort();
        std::for_each(d->listeners.begin(), d->listeners.end(), [&](MidiInputDeviceCallback *cb) {
            cb->deviceStoppedCallback();
        });
        setErrorString({});
    }

    void MidiInputDevice::addListener(MidiInputDeviceCallback *callback) {
        d->listeners.append(callback);
    }

    void MidiInputDevice::removeListener(MidiInputDeviceCallback *callback) {
        d->listeners.removeOne(callback);
    }

    QStringList MidiInputDevice::devices() {
        QStringList a;
        try {
            RtMidiIn tmp;
            int count = tmp.getPortCount();
            a.reserve(count);
            for (int i = 0; i < count; i++) {
                a.append(tmp.getPortName(i).c_str());
            }
        } catch (RtMidiError &e) {
            qWarning() << "MidiInputDevice:" << e.getMessage().c_str();
        }
        return a;
    }
} // talcs