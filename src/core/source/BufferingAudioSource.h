#ifndef TALCS_BUFFERINGAUDIOSOURCE_H
#define TALCS_BUFFERINGAUDIOSOURCE_H

#include <TalcsCore/PositionableAudioSource.h>
#include <QDeadlineTimer>

class QThreadPool;

namespace talcs {

    class BufferingAudioSourcePrivate;

    class TALCSCORE_EXPORT BufferingAudioSource : public PositionableAudioSource {
        Q_DECLARE_PRIVATE(BufferingAudioSource)
    public:
        explicit BufferingAudioSource(PositionableAudioSource *src, int channelCount, qint64 readAheadSize,
                                      bool autoBuffering = true, QThreadPool *threadPool = nullptr);
        explicit BufferingAudioSource(PositionableAudioSource *src, bool takeOwnership, int channelCount,
                                      qint64 readAheadSize, bool autoBuffering = true, QThreadPool *threadPool = nullptr);

        ~BufferingAudioSource() override;

        qint64 read(const AudioSourceReadData &readData) override;
        qint64 length() const override;
        void setNextReadPosition(qint64 pos) override;

        bool open(qint64 bufferSize, double sampleRate) override;
        void close() override;

        void setReadAheadSize(qint64 size);
        qint64 readAheadSize() const;

        PositionableAudioSource *source() const;

        bool waitForBuffering(QDeadlineTimer deadline = QDeadlineTimer::Forever);

        void flush();
        void startBuffering();

        static QThreadPool *threadPool();

    protected:
        explicit BufferingAudioSource(BufferingAudioSourcePrivate &d);
    };

} // talcs

#endif //TALCS_BUFFERINGAUDIOSOURCE_H
