#ifndef AUDIOFORMATINPUTSOURCE_H
#define AUDIOFORMATINPUTSOURCE_H

#include <TalcsCore/PositionableAudioSource.h>
#include <TalcsFormat/TalcsFormatGlobal.h>

namespace talcs {

    class AudioFormatInputSourcePrivate;
    class AudioFormatIO;

    class TALCSFORMAT_EXPORT AudioFormatInputSource : public PositionableAudioSource {
        Q_DECLARE_PRIVATE(AudioFormatInputSource);
    public:
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

    protected:
        explicit AudioFormatInputSource(AudioFormatInputSourcePrivate &d);
    };
    
}

#endif // AUDIOFORMATINPUTSOURCE_H