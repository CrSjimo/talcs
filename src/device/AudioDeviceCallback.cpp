#include "AudioDeviceCallback.h"

namespace talcs {
    /**
     * @class AudioDeviceCallback
     * @brief Callback functions of audio device
     * @see AudioDevice
     */

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