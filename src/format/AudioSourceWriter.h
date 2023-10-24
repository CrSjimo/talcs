#ifndef TALCS_AUDIOSOURCEWRITER_H
#define TALCS_AUDIOSOURCEWRITER_H

#include "core/base/AudioSourceProcessorBase.h"

namespace talcs {
    class AudioSourceWriterPrivate;
    class AudioSource;
    class AudioFormatIO;

    class TALCS_EXPORT AudioSourceWriter : public AudioSourceProcessorBase {
        Q_OBJECT
        Q_DECLARE_PRIVATE(AudioSourceWriter)
    public:
        AudioSourceWriter(AudioSource *src, AudioFormatIO *outFile, qint64 length, QObject *parent = nullptr);
        ~AudioSourceWriter() override;

    protected:
        IAudioSampleContainer *prepareBuffer() override;
        bool processBlock(qint64 processedSampleCount, qint64 samplesToProcess) override;
        void processWillFinish() override;
    };
}



#endif // TALCS_AUDIOSOURCEWRITER_H
