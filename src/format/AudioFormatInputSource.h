#ifndef TALCS_AUDIOFORMATINPUTSOURCE_H
#define TALCS_AUDIOFORMATINPUTSOURCE_H

#include "core/source/PositionableAudioSource.h"

namespace talcs {
    class AudioFormatInputSourcePrivate;
    class AudioFormatIO;

    class TALCS_EXPORT AudioFormatInputSource : public PositionableAudioSource {
        Q_DECLARE_PRIVATE(AudioFormatInputSource);
    public:
        enum ResampleMode {
            SincBestQuality = 0,
            SincMediumQuality,
            SincFastest,
            ZeroOrderHold,
            Linear,
        };
        explicit AudioFormatInputSource(AudioFormatIO *audioFormatIo = nullptr);
        ~AudioFormatInputSource() override;

        qint64 read(const AudioSourceReadData &readData) override;
        qint64 length() const override;
        void setNextReadPosition(qint64 pos) override;
        bool open(qint64 bufferSize, double sampleRate) override;
        void close() override;

        void setAudioFormatIo(AudioFormatIO *audioFormatIo);
        AudioFormatIO *audioFormatIo() const;

        void flush();

        void setStereoize(bool stereoize);
        bool stereoize() const;

        void setResamplerMode(ResampleMode mode);
        ResampleMode resampleMode() const;

    protected:
        explicit AudioFormatInputSource(AudioFormatInputSourcePrivate &d);
    };
}

#endif // TALCS_AUDIOFORMATINPUTSOURCE_H
