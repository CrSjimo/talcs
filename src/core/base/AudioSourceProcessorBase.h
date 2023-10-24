#ifndef TALCS_AUDIOSOURCEPROCESSORBASE_H
#define TALCS_AUDIOSOURCEPROCESSORBASE_H

#include <QObject>

#include "global/TalcsGlobal.h"

namespace talcs {

    class AudioSourceProcessorBasePrivate;
    class AudioSource;
    class IAudioSampleContainer;

    class TALCS_EXPORT AudioSourceProcessorBase: public QObject {
        Q_OBJECT
        Q_DECLARE_PRIVATE(AudioSourceProcessorBase)
    public:
        AudioSourceProcessorBase(AudioSource *src, qint64 length, QObject *parent = nullptr);
        ~AudioSourceProcessorBase() override;

        enum Status {
            Ready,
            Processing,
            Failed,
            Interrupted,
            Completed,
        };
        Status status() const;

        AudioSource *source() const;
        qint64 length() const;
        void interrupt();

    public slots:
        void start();

    signals:
        void blockProcessed(qint64 processedSampleCount);
        void finished();

    protected:
        AudioSourceProcessorBase(AudioSourceProcessorBasePrivate &d, AudioSource *src, qint64 length, QObject *parent);
        virtual IAudioSampleContainer *prepareBuffer() = 0;
        virtual bool processBlock(qint64 processedSampleCount, qint64 samplesToProcess) = 0;
        virtual void processWillFinish() = 0;

        QScopedPointer<AudioSourceProcessorBasePrivate> d_ptr;
    };

} // talcs

#endif // TALCS_AUDIOSOURCEPROCESSORBASE_H
