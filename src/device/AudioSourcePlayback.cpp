#include "AudioSourcePlayback.h"
#include "AudioSourcePlayback_p.h"

#include "AudioDevice.h"
#include "core/source/AudioSource.h"

namespace talcs {
    /**
     * @class AudioSourcePlayback
     * @brief Callback functions that starts, stops and reads data from an AudioSource
     *
     * In practice, TransportAudioSource is usually used to control the transportation of audio.
     * @see AudioSource, TransportAudioSource
     */

    /**
     * Constructor.
     * @param src the AudioSource to process
     * @param takeOwnership If the ownership of the AudioSource object is taken, the object will be deleted on
     * destruction.
     */
    AudioSourcePlayback::AudioSourcePlayback(AudioSource *src, bool takeOwnership)
        : AudioSourcePlayback(*new AudioSourcePlaybackPrivate) {
        Q_D(AudioSourcePlayback);
        d->src = src;
        d->takeOwnership = takeOwnership;
    }

    AudioSourcePlayback::AudioSourcePlayback(AudioSourcePlaybackPrivate &d) : d_ptr(&d) {
        d.q_ptr = this;
    }

    /**
     * Destructor.
     *
     * If the ownership of the AudioSource object is taken, it will be deleted.
     */
    AudioSourcePlayback::~AudioSourcePlayback() {
        Q_D(AudioSourcePlayback);
        if (d->takeOwnership) {
            delete d->src;
        }
    }

    /**
     * Gets the AudioSource object used.
     */
    AudioSource *AudioSourcePlayback::source() const {
        Q_D(const AudioSourcePlayback);
        return d->src;
    }

    /**
     * Change the AudioSource object used dynamically. The ownership of the previous object is no more taken.
     */
    void AudioSourcePlayback::setSource(AudioSource *src, bool takeOwnership) {
        Q_D(AudioSourcePlayback);
        d->src = src;
        d->takeOwnership = takeOwnership;
    }

    void AudioSourcePlayback::deviceWillStartCallback(AudioDevice *device) {
        Q_D(AudioSourcePlayback);
        d->src->open(device->bufferSize(), device->sampleRate());
    }
    void AudioSourcePlayback::deviceStoppedCallback() {
        Q_D(AudioSourcePlayback);
        d->src->close();
    }

    void AudioSourcePlayback::workCallback(const AudioSourceReadData &readData) {
        Q_D(AudioSourcePlayback);
        d->src->read(readData);
    }
}
