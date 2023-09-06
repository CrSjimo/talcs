#ifndef TALCS_SINEWAVEAUDIOSOURCE_H
#define TALCS_SINEWAVEAUDIOSOURCE_H

#include "PositionableAudioSource.h"

#include <functional>

namespace talcs {
    class SineWaveAudioSourcePrivate;

    class TALCS_EXPORT SineWaveAudioSource : public PositionableAudioSource {
        Q_DECLARE_PRIVATE(SineWaveAudioSource)
    public:
        explicit SineWaveAudioSource(double frequency);
        explicit SineWaveAudioSource(const std::function<double(qint64)> &frequencyIntegration);
        ~SineWaveAudioSource() override = default;

        bool open(qint64 bufferSize, double sampleRate) override;
        qint64 read(const AudioSourceReadData &readData) override;
        qint64 length() const override;

        void setFrequency(double frequency);
        void setFrequency(const std::function<double(qint64)> &frequencyIntegration);
        std::function<double(qint64)> frequency() const;

    protected:
        explicit SineWaveAudioSource(SineWaveAudioSourcePrivate &d);
    };
}

#endif // TALCS_SINEWAVEAUDIOSOURCE_H
