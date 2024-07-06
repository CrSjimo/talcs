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

#ifndef TALCS_MIDIINPUTDEVICE_P_H
#define TALCS_MIDIINPUTDEVICE_P_H

#include <TalcsMidi/MidiInputDevice.h>

#include <rtmidi/RtMidi.h>

#include <TalcsMidi/MidiMessageListener.h>

class RtMidiIn;

namespace talcs {

    class MidiInputDeviceRootListener : public MidiMessageListener {
    protected:
        bool processDeviceWillStart(MidiInputDevice *device) override {
            return true;
        }

        void processDeviceStopped() override {
        }

        bool processMessage(const MidiMessage &message) override {
            return false;
        }

        void processError(const QString &errorString) override {

        }
    };

    class MidiInputDevicePrivate {
        Q_DECLARE_PUBLIC(MidiInputDevice)
    public:
        MidiInputDevice *q_ptr;
        int portNumber;
        QScopedPointer<RtMidiIn> midi;
        mutable MidiInputDeviceRootListener listener;

        static void rtmidiCallback(double timeStamp, std::vector<unsigned char> *message, void *userData);
        static void rtmidiErrorCallback(RtMidiError::Type type, const std::string &errorText, void *userData);
    };
}

#endif //TALCS_MIDIINPUTDEVICE_P_H
