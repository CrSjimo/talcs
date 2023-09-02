#ifndef TALCS_SINEWAVEAUDIOSOURCE_H
#define TALCS_SINEWAVEAUDIOSOURCE_H

#include "PositionableAudioSource.h"

namespace talcs {
    class SineWaveAudioSourcePrivate;

    class TALCS_EXPORT SineWaveAudioSource : public PositionableAudioSource {
        Q_DECLARE_PRIVATE(SineWaveAudioSource)
    public:
        SineWaveAudioSource();
        explicit SineWaveAudioSource(double frequency);
        ~SineWaveAudioSource() override = default;
        bool open(qint64 bufferSize, double sampleRate) override;
        qint64 read(const AudioSourceReadData &readData) override;
        qint64 length() const override;

        void setFrequency(double frequency);
        double frequency() const;

    protected:
        explicit SineWaveAudioSource(SineWaveAudioSourcePrivate &d);
    };
}

#endif // TALCS_SINEWAVEAUDIOSOURCE_H
