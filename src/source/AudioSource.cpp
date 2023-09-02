#include "AudioSource.h"
#include "AudioSource_p.h"

#include "buffer/AudioBuffer.h"

namespace talcs {
    AudioSourceReadData::AudioSourceReadData(IAudioSampleContainer * buffer)
        : buffer(buffer), startPos(0), length(buffer->sampleCount()) {
    }

    AudioSourceReadData::AudioSourceReadData(IAudioSampleContainer * buffer, qint64 startPos, qint64 length)
        : buffer(buffer), startPos(startPos), length(length) {
    }

    AudioSource::AudioSource() : AudioSource(*new AudioSourcePrivate) {
    }

    AudioSource::AudioSource(AudioSourcePrivate & d) : d_ptr(&d) {
        d.q_ptr = this;
    }
    AudioSource::~AudioSource() {
    }
}
