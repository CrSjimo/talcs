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

#include <cmath>
#include <utility>
#include <vector>

#ifdef Q_OS_WIN
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <Windows.h>
#  include <mmsystem.h>
#endif

#include <QDebug>
#include <QMetaObject>
#include <QThread>

#include <TalcsMidi/MidiMessageListener.h>

namespace talcs {

#ifdef Q_OS_WIN
    namespace {

        bool isWinMMInputUnavailable() {
            std::vector<RtMidi::Api> apis;
            RtMidi::getCompiledApi(apis);
            // WinMM emits the empty-device warning before an error callback can be installed.
            return apis.size() == 1 && apis.front() == RtMidi::WINDOWS_MM && midiInGetNumDevs() == 0;
        }

    }
#endif

    MidiInputDevice::MidiInputDevice(int deviceIndex, QObject *parent) : QObject(parent), d_ptr(new MidiInputDevicePrivate) {
        Q_D(MidiInputDevice);
        d->q_ptr = this;
        d->portNumber = deviceIndex;
        d->sourceId = MidiInputEvent::createSourceId();
#ifdef Q_OS_WIN
        if (isWinMMInputUnavailable()) {
            setErrorString(tr("The MIDI input device is unavailable."));
            return;
        }
#endif
        try {
            d->midi.reset(new RtMidiIn);
            setName(QString::fromStdString(d->midi->getPortName(deviceIndex)));
        } catch (RtMidiError &e) {
            setErrorString(QString::fromStdString(e.getMessage()));
        }
    }

    MidiInputDevice::~MidiInputDevice() {
        Q_D(MidiInputDevice);
        close();
        d->midi.reset();
    }

    void MidiInputDevicePrivate::rtmidiCallback(double timeStamp, std::vector<unsigned char> *message, void *userData) {
        auto d = static_cast<MidiInputDevicePrivate *>(userData);
        if (d->closing.load() || !message || message->empty())
            return;
        if (!d->timeStampInitialized || !std::isfinite(timeStamp) || timeStamp < 0) {
            d->timeStamp = MidiInputEvent::currentTime();
            d->timeStampInitialized = true;
        } else {
            d->timeStamp += timeStamp;
        }
        MidiMessage msg(message->data(), static_cast<int>(message->size()), d->timeStamp);
        if (msg.isValid())
            d->listener.eventCallback({msg, d->sourceId});
    }

    void MidiInputDevicePrivate::rtmidiErrorCallback(RtMidiError::Type type, const std::string &errorText, void *userData) {
        if (type == RtMidiError::WARNING || type == RtMidiError::DEBUG_WARNING)
            return;
        auto d = static_cast<MidiInputDevicePrivate *>(userData);
        if (d->closing.load())
            return;
        const auto error = QString::fromStdString(errorText);
        const auto reportError = [device = d->q_ptr, error] { device->setErrorString(error); };
        if (QThread::currentThread() == d->q_ptr->thread())
            reportError();
        else
            QMetaObject::invokeMethod(d->q_ptr, reportError, Qt::QueuedConnection);
        d->listener.eventCallback({MidiMessage().withTimeStamp(MidiInputEvent::currentTime()), d->sourceId, MidiInputEvent::SourceStopped});
        d->listener.errorCallback(QString::fromStdString(errorText));
    }

    int MidiInputDevice::deviceIndex() const {
        Q_D(const MidiInputDevice);
        return d->portNumber;
    }

    quint64 MidiInputDevice::sourceId() const {
        Q_D(const MidiInputDevice);
        return d->sourceId;
    }

    bool MidiInputDevice::open() {
        Q_D(MidiInputDevice);
        if (!d->midi)
            return false;
        if (isOpen())
            return true;
        d->closing.store(false);
        try {
            if (d->portNumber < 0 || static_cast<unsigned>(d->portNumber) >= d->midi->getPortCount()) {
                setErrorString(tr("The MIDI input device is unavailable."));
                return false;
            }
            d->listenerStarted = true;
            if (!d->listener.deviceWillStartCallback(this)) {
                close();
                setErrorString(tr("The MIDI input listener could not start."));
                return false;
            }
            d->timeStampInitialized = false;
            d->midi->ignoreTypes(false, false, false);
            d->midi->setCallback(&MidiInputDevicePrivate::rtmidiCallback, d);
            d->callbackRegistered = true;
            d->midi->setErrorCallback(&MidiInputDevicePrivate::rtmidiErrorCallback, d);
            d->midi->openPort(d->portNumber);
            if (!d->midi->isPortOpen()) {
                const auto error = errorString();
                close();
                setErrorString(error.isEmpty() ? tr("The MIDI input device could not be opened.") : error);
                return false;
            }
        } catch (RtMidiError &e) {
            close();
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
        if (d->closing.exchange(true))
            return;
        QString closeError;
        if (d->midi) {
            try {
                // Port shutdown must complete before callback registration changes.
                d->midi->closePort();
                if (d->callbackRegistered) {
                    d->midi->cancelCallback();
                    d->callbackRegistered = false;
                }
            } catch (const RtMidiError &error) {
                closeError = QString::fromStdString(error.getMessage());
            }
        }
        if (std::exchange(d->listenerStarted, false)) {
            d->listener.eventCallback({MidiMessage().withTimeStamp(MidiInputEvent::currentTime()), d->sourceId, MidiInputEvent::SourceStopped});
            d->listener.deviceStoppedCallback();
        }
        setErrorString(closeError);
    }

    MidiMessageListener *MidiInputDevice::listener() const {
        Q_D(const MidiInputDevice);
        return &d->listener;
    }

    QStringList MidiInputDevice::devices() {
#ifdef Q_OS_WIN
        if (isWinMMInputUnavailable())
            return {};
#endif
        QStringList a;
        try {
            RtMidiIn tmp;
            unsigned count = tmp.getPortCount();
            a.reserve(count);
            for (unsigned i = 0; i < count; i++) {
                a.append(QString::fromStdString(tmp.getPortName(i)));
            }
        } catch (RtMidiError &e) {
            qWarning() << "MidiInputDevice:" << QString::fromStdString(e.getMessage());
        }
        return a;
    }
} // talcs
