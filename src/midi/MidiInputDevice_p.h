#ifndef TALCS_MIDIINPUTDEVICE_P_H
#define TALCS_MIDIINPUTDEVICE_P_H

#include <TalcsMidi/MidiInputDevice.h>

class RtMidiIn;

namespace talcs {
    class MidiInputDevicePrivate {
    public:
        int portNumber;
        QScopedPointer<RtMidiIn> midi;
        QList<MidiInputDeviceCallback *> listeners;
    };
}

#endif //TALCS_MIDIINPUTDEVICE_P_H
