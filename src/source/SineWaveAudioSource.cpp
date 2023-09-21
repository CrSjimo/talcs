#include "SineWaveAudioSource.h"
#include "SineWaveAudioSource_p.h"

#include <cmath>

namespace talcs {
    /**
     * @class SineWaveAudioSource
     * Generates sine wave.
     */

    /**
     * @overload
     *
     * Constructor that initialize the generator with a fixed frequency (Hz).
     */
    SineWaveAudioSource::SineWaveAudioSource(double frequency): PositionableAudioSource(*new SineWaveAudioSourcePrivate) {
        Q_D(SineWaveAudioSource);
        setFrequency(frequency);
    }

    /**
     * Constructor that initialize the generator with a frequency that changes over time.
     * @param frequencyIntegration @f$\mathtt{frequencyIntegration}(x) = \int_{0}^{x}f(\tau)\mathrm{d}\tau@f$,
     * where @f$f(\tau)@f$ represents the frequency (Hz) at time @f$\tau@f$ (measured in samples).
     */
    SineWaveAudioSource::SineWaveAudioSource(const std::function<double(qint64)> &frequencyIntegration): PositionableAudioSource(*new SineWaveAudioSourcePrivate) {
        Q_D(SineWaveAudioSource);
        setFrequency(frequencyIntegration);
    }

    bool SineWaveAudioSource::open(qint64 bufferSize, double sampleRate) {
        return AudioSource::open(bufferSize, sampleRate);
    }

    qint64 SineWaveAudioSource::read(const AudioSourceReadData &readData) {
        Q_D(SineWaveAudioSource);
        QMutexLocker locker(&d->mutex);
        static const double PI = 3.14159265358979323846;
        auto channelCount = readData.buffer->channelCount();
        auto pos = d->position;
        auto sr = sampleRate();
        for (int ch = 0; ch < channelCount; ch++) {
            if ((1 << ch) & readData.silentFlags)
                readData.buffer->clear(ch, readData.startPos, readData.length);
        }
        if (readData.silentFlags != -1) {
            for (qint64 i = 0; i < readData.length; i++) {
                float sample = sin(2 * PI * d->freq(pos + i) / sr);
                for (int ch = 0; ch < channelCount; ch++) {
                    if ((1 << ch) & readData.silentFlags)
                        continue;
                    readData.buffer->sampleAt(ch, readData.startPos + i) = sample;
                }
            }
        }
        d->position += readData.length;
        return readData.length;
    }
    qint64 SineWaveAudioSource::length() const {
        return std::numeric_limits<qint64>::max();
    }

    /**
     * @overload
     *
     * Sets a fixed frequency.
     * @see SineWaveAudioSource(double)
     */
    void SineWaveAudioSource::setFrequency(double frequency) {
        setFrequency([frequency](qint64 pos) { return frequency * pos; });
    }

    /**
     * Sets a frequency that changes over time.
     * @see SineWaveAudioSource(const std::function<double(qint64)> &)
     */
    void SineWaveAudioSource::setFrequency(const std::function<double(qint64)> &frequencyIntegration) {
        Q_D(SineWaveAudioSource);
        QMutexLocker locker(&d->mutex);
        d->freq = frequencyIntegration;
    }

    /**
     * Gets the frequency integration function.
     * @see SineWaveAudioSource(const std::function<double(qint64)> &)
     */
    std::function<double(qint64)> SineWaveAudioSource::frequency() const {
        Q_D(const SineWaveAudioSource);
        return d->freq;
    }

    void SineWaveAudioSource::setNextReadPosition(qint64 pos) {
        Q_D(SineWaveAudioSource);
        QMutexLocker locker(&d->mutex);
        PositionableAudioSource::setNextReadPosition(pos);
    }
}