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
