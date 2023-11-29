#ifndef TALCS_MIDIINPUTDEVICECALLBACK_H
#define TALCS_MIDIINPUTDEVICECALLBACK_H

#include <TalcsMidi/MidiMessage.h>

namespace talcs {

    class MidiInputDevice;

    class TALCSMIDI_EXPORT MidiInputDeviceCallback {
    public:
        virtual void deviceWillStartCallback(MidiInputDevice *device) = 0;
        virtual void deviceStoppedCallback() = 0;
        virtual void workCallback(const MidiMessage &message) = 0;
        virtual void errorCallback(const QString &errorString) = 0;
    };

} // talcs

#endif //TALCS_MIDIINPUTDEVICECALLBACK_H
