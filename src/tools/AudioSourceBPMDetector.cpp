#include "AudioSourceBPMDetector.h"
#include "AudioSourceBPMDetector_p.h"

#include "source/AudioSource.h"

namespace talcs {
    AudioSourceBPMDetector::AudioSourceBPMDetector(AudioSource *src, qint64 length, QObject *parent)
        : AudioSourceProcessorBase(*new AudioSourceProcessorBasePrivate, src, length, parent) {
        Q_D(AudioSourceBPMDetector);
        d->bpmDetect.reset(new soundtouch::BPMDetect(1, src->sampleRate()));
        d->buf.resize(1, src->bufferSize());
    }

    AudioSourceBPMDetector::~AudioSourceBPMDetector() = default;

    IAudioSampleContainer *AudioSourceBPMDetector::prepareBuffer() {
        Q_D(AudioSourceBPMDetector);
        return &d->buf;
    }
    bool AudioSourceBPMDetector::processBlock(qint64 processedSampleCount, qint64 samplesToProcess) {
        Q_D(AudioSourceBPMDetector);
        d->bpmDetect->inputSamples(d->buf.constData(0), samplesToProcess);
        return true;
    }
    void AudioSourceBPMDetector::processWillFinish() {
    }

    float AudioSourceBPMDetector::bpm() const {
        Q_D(const AudioSourceBPMDetector);
        return d->bpmDetect->getBpm();
    }
} // talcs