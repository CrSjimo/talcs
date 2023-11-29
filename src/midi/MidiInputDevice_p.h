#ifndef TALCS_MIDIINPUTDEVICE_P_H
#define TALCS_MIDIINPUTDEVICE_P_H

#include <TalcsMidi/MidiInputDevice.h>

class RtMidiIn;

namespace talcs {
    class MidiInputDevicePrivate {
    public:
        int portNumber;
        QScopedPointer<RtMidiIn> midi;
        MidiInputDeviceCallback *callback = nullptr;
    };
}

#endif //TALCS_MIDIINPUTDEVICE_P_H
