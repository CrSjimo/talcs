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

#ifndef TALCS_AUDIODEVICECALLBACK_H
#define TALCS_AUDIODEVICECALLBACK_H

#include <TalcsDevice/TalcsDeviceGlobal.h>

namespace talcs {
    
    class AudioDevice;
    class AudioBuffer;
    struct AudioSourceReadData;

    class TALCSDEVICE_EXPORT AudioDeviceCallback {
    public:

        virtual ~AudioDeviceCallback();

        virtual bool deviceWillStartCallback(AudioDevice *device) = 0;
        virtual void deviceStoppedCallback() = 0;
        virtual void workCallback(const AudioSourceReadData &readData) = 0;
    };

}
#endif // TALCS_AUDIODEVICECALLBACK_H
