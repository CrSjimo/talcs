#ifndef TALCS_SDLAUDIODEVICE_P_H
#define TALCS_SDLAUDIODEVICE_P_H

#include <SDL2/SDL.h>

#include "AudioDevice_p.h"
#include "SDLAudioDevice.h"

namespace talcs {
    class SDLAudioDevicePrivate : public AudioDevicePrivate {
        Q_DECLARE_PUBLIC(SDLAudioDevice)
    public:
        quint32 devId = 0;
        AudioDeviceCallback *audioDeviceCallback = nullptr;
        SDL_AudioSpec spec = {};

        void sdlCallback(quint8 *rawBuf, int length);
    };
}

#endif // TALCS_SDLAUDIODEVICE_P_H
