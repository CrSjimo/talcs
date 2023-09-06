#ifndef TALCS_TRANSPORTAUDIOSOURCEWRITER_H
#define TALCS_TRANSPORTAUDIOSOURCEWRITER_H

#include <QObject>

#include "global/TalcsGlobal.h"

namespace talcs {
    class TransportAudioSourceWriterPrivate;
    class TransportAudioSource;
    class AudioFormatIO;

    class TALCS_EXPORT TransportAudioSourceWriter : public QObject {
        Q_OBJECT
        Q_DECLARE_PRIVATE(TransportAudioSourceWriter)
    public:
        TransportAudioSourceWriter(TransportAudioSource *src, AudioFormatIO *outFile, qint64 startPos, qint64 length);
        ~TransportAudioSourceWriter() override;

    public slots:
        void start();
        void interrupt();

    signals:
        void percentageUpdated(float percentage);

        void completed();
        void interrupted();
        void finished(bool isCompleted);

    protected:
        QScopedPointer<TransportAudioSourceWriterPrivate> d_ptr;
        explicit TransportAudioSourceWriter(TransportAudioSourceWriterPrivate &d);
    };
}



#endif // TALCS_TRANSPORTAUDIOSOURCEWRITER_H
