#ifndef TALCS_AUDIODEVICECALLBACK_H
#define TALCS_AUDIODEVICECALLBACK_H

#include <TalcsDevice/TalcsDeviceGlobal.h>

namespace talcs {
    
    class AudioDevice;
    class AudioBuffer;
    class AudioSourceReadData;

    class TALCSDEVICE_EXPORT AudioDeviceCallback {
    public:
        virtual void deviceWillStartCallback(AudioDevice *device) = 0;
        virtual void deviceStoppedCallback() = 0;
        virtual void workCallback(const AudioSourceReadData &readData) = 0;
    };

}
#endif // TALCS_AUDIODEVICECALLBACK_H
