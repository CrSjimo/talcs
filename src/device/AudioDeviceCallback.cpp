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

#include "AudioDeviceCallback.h"

namespace talcs {

    /**
     * @class AudioDeviceCallback
     * @brief Callback functions of audio device
     * @see AudioDevice
     */

    AudioDeviceCallback::~AudioDeviceCallback() = default;

    /**
     * @fn void AudioDeviceCallback::deviceWillStartCallback(AudioDevice *device)
     * Called when device is about to start.
     * @param device the target audio device
     */

    /**
     * @fn void AudioDeviceCallback::deviceStoppedCallback()
     * Called when device is stopped.
     */

    /**
     * @fn void AudioDeviceCallback::workCallback(const AudioSourceReadData &readData)
     * Called on each frame of audio streaming.
     */
     
}