#include "SineWaveAudioSource.h"
#include "SineWaveAudioSource_p.h"

#include <cmath>

namespace talcs {
    SineWaveAudioSource::SineWaveAudioSource() : SineWaveAudioSource(*new SineWaveAudioSourcePrivate) {
    }
    SineWaveAudioSource::SineWaveAudioSource(double frequency) : SineWaveAudioSource() {
        Q_D(SineWaveAudioSource);
        setFrequency(frequency);
    }
    SineWaveAudioSource::SineWaveAudioSource(const std::function<double(qint64)> &getFreq) : SineWaveAudioSource() {
        Q_D(SineWaveAudioSource);
        setFrequency(getFreq);
    }
    SineWaveAudioSource::SineWaveAudioSource(SineWaveAudioSourcePrivate & d) : PositionableAudioSource(d) {
    }

    bool SineWaveAudioSource::open(qint64 bufferSize, double sampleRate) {
        return AudioSource::open(bufferSize, sampleRate);
    }

    qint64 SineWaveAudioSource::read(const AudioSourceReadData &readData) {
        Q_D(SineWaveAudioSource);
        QMutexLocker locker(&d->mutex);
        auto channelCount = readData.buffer->channelCount();
        auto pos = PositionableAudioSource::nextReadPosition();
        for (qint64 i = 0; i < readData.length; i++) {
            double omega = 2 * 3.14159265358979323846 * d->freq(pos + i) / sampleRate();
            float sample = sin(omega * (pos + i));
            for (int ch = 0; ch < channelCount; ch++) {
                readData.buffer->sampleAt(ch, readData.startPos + i) = sample;
            }
        }
        setNextReadPosition(pos + readData.length);
        return readData.length;
    }
    qint64 SineWaveAudioSource::length() const {
        return std::numeric_limits<qint64>::max();
    }

    void SineWaveAudioSource::setFrequency(double frequency) {
        setFrequency([frequency](qint64 _){ return frequency; });
    }
    void SineWaveAudioSource::setFrequency(const std::function<double(qint64)> &getFreq) {
        Q_D(SineWaveAudioSource);
        QMutexLocker locker(&d->mutex);
        d->freq = getFreq;
    }
    std::function<double(qint64)> SineWaveAudioSource::frequency() const {
        Q_D(const SineWaveAudioSource);
        return d->freq;
    }
}