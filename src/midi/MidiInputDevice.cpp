#include "MidiInputDevice.h"
#include "MidiInputDevice_p.h"

#include <rtmidi/RtMidi.h>

#include <QDebug>

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

    static void rtmidiCallback(double timeStamp, std::vector<unsigned char> *message, void *userData) {
        auto d = reinterpret_cast<MidiInputDevicePrivate *>(userData);
        MidiMessage msg(message->data(), message->size(), timeStamp);
        d->callback->workCallback(msg);
    }

    static void rtmidiErrorCallback(RtMidiError::Type type, const std::string &errorText, void *userData) {
        Q_UNUSED(type);
        auto d = reinterpret_cast<MidiInputDevicePrivate *>(userData);
        d->callback->errorCallback(errorText.c_str());
    }

    bool MidiInputDevice::open(MidiInputDeviceCallback *callback) {
        if (!d->midi)
            return false;
        try {
            d->midi->openPort(d->portNumber);
            d->callback = callback;
            d->callback->deviceWillStartCallback(this);
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
        if (d->callback) {
            d->callback->deviceStoppedCallback();
            d->callback = nullptr;
        }
        setErrorString({});
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