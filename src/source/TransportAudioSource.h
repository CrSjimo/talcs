#ifndef TRANSPORTAUDIOSOURCE_H
#define TRANSPORTAUDIOSOURCE_H

#include <QObject>

#include "AudioSource.h"
#include "PositionableAudioSource.h"

namespace talcs {
    class TransportAudioSourcePrivate;
    class TransportAudioSourceWriter;

    class TALCS_EXPORT TransportAudioSource : public QObject, public AudioSource {
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

        void play();
        bool isPlaying() const;
        void pause();

        qint64 position() const;
        void setPosition(qint64 position);
        qint64 length() const;

        QPair<qint64, qint64> loopingRange() const;
        void setLoopingRange(qint64 l, qint64 r);

    signals:
        void positionAboutToChange(qint64 nextPosition);

    protected:
        TransportAudioSource(TransportAudioSourcePrivate &d, QObject *parent);
    };

    class TransportAudioSourceStateSaverPrivate;

    class TALCS_EXPORT TransportAudioSourceStateSaver {
    public:
        explicit TransportAudioSourceStateSaver(TransportAudioSource *src);
        ~TransportAudioSourceStateSaver();

    private:
        QScopedPointer<TransportAudioSourceStateSaverPrivate> d;
    };

}

#endif // TRANSPORTAUDIOSOURCE_H
