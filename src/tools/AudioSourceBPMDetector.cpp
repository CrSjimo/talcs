#include "AudioSourceBPMDetector.h"
#include "AudioSourceBPMDetector_p.h"

#include "source/AudioSource.h"

#include <QDebug>

namespace talcs {

    AudioSourceBPMDetectorPrivate::AudioSourceBPMDetectorPrivate(AudioSource *src)
        : bpmDetect(1, src->sampleRate()),
          buf(1, src->bufferSize()) {
    }

    AudioSourceBPMDetector::AudioSourceBPMDetector(AudioSource *src, qint64 length, QObject *parent)
        : AudioSourceProcessorBase(*new AudioSourceBPMDetectorPrivate(src), src, length, parent) {
        Q_D(AudioSourceBPMDetector);
    }

    AudioSourceBPMDetector::~AudioSourceBPMDetector() = default;

    IAudioSampleContainer *AudioSourceBPMDetector::prepareBuffer() {
        Q_D(AudioSourceBPMDetector);
        return &d->buf;
    }
    bool AudioSourceBPMDetector::processBlock(qint64 processedSampleCount, qint64 samplesToProcess) {
        Q_D(AudioSourceBPMDetector);
        d->bpmDetect.inputSamples(d->buf.constData(0), samplesToProcess);
        qDebug() << bpm();
        return true;
    }
    void AudioSourceBPMDetector::processWillFinish() {
    }

    float AudioSourceBPMDetector::bpm() const {
        Q_D(const AudioSourceBPMDetector);
        return const_cast<AudioSourceBPMDetectorPrivate *>(d)->bpmDetect.getBpm();
    }
} // talcs