#ifndef TALCS_TRANSPORTAUDIOSOURCE_H
#define TALCS_TRANSPORTAUDIOSOURCE_H

#include <QObject>

#include <TalcsCore/AudioSource.h>
#include <TalcsCore/PositionableAudioSource.h>

namespace talcs {

    class TransportAudioSourcePrivate;
    class TransportAudioSourceWriter;

    class TALCSCORE_EXPORT TransportAudioSource : public QObject, public AudioSource {
        Q_OBJECT
        Q_DECLARE_PRIVATE_D(AudioSource::d_ptr, TransportAudioSource)
        friend class TransportAudioSourceWriter;

    public:
        explicit TransportAudioSource(QObject *parent = nullptr);
        explicit TransportAudioSource(PositionableAudioSource *src, bool takeOwnership = false, QObject *parent = nullptr);
        ~TransportAudioSource() override;
        qint64 read(const AudioSourceReadData &readData) override;
        bool open(qint64 bufferSize, double sampleRate) override;
        void close() override;

        void setSource(PositionableAudioSource *src, bool takeOwnership = false);
        PositionableAudioSource *source() const;

        bool isPlaying() const;

        qint64 position() const;
        void setPosition(qint64 position);
        qint64 length() const;

        QPair<qint64, qint64> loopingRange() const;
        void setLoopingRange(qint64 l, qint64 r);

        int bufferingCounter() const;

    public slots:
        void play();
        void pause();
        void acquireBuffering();
        void releaseBuffering();

    signals:
        void positionAboutToChange(qint64 nextPosition);
        void bufferingCounterChanged(int counter);

    protected:
        TransportAudioSource(TransportAudioSourcePrivate &d, QObject *parent);
    };

    class TransportAudioSourceStateSaverPrivate;

    class TALCSCORE_EXPORT TransportAudioSourceStateSaver {
    public:
        explicit TransportAudioSourceStateSaver(TransportAudioSource *src);
        ~TransportAudioSourceStateSaver();

    private:
        QScopedPointer<TransportAudioSourceStateSaverPrivate> d;
    };

}

#endif // TALCS_TRANSPORTAUDIOSOURCE_H
