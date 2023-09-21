#ifndef TALCS_FUTUREAUDIOSOURCE_H
#define TALCS_FUTUREAUDIOSOURCE_H

#include <QFuture>

#include "source/PositionableAudioSource.h"

namespace talcs {

    struct FutureAudioSourceCallbacks {
        std::function<bool (qint64, double)> preloadingOpen = [](qint64, double){ return true; };
        std::function<void ()> preloadingClose;
    };

    class FutureAudioSourcePrivate;

    class TALCS_EXPORT FutureAudioSource: public QObject, public PositionableAudioSource {
        Q_OBJECT
        Q_DECLARE_PRIVATE_D(PositionableAudioSource::d_ptr, FutureAudioSource)
    public:
        explicit FutureAudioSource(const QFuture<PositionableAudioSource *> &future, const FutureAudioSourceCallbacks &callbacks = {}, QObject *parent = nullptr);
        ~FutureAudioSource() override;

        QFuture<PositionableAudioSource *> future() const;

        qint64 read(const AudioSourceReadData &readData) override;
        qint64 length() const override;
        qint64 nextReadPosition() const override;
        void setNextReadPosition(qint64 pos) override;
        bool open(qint64 bufferSize, double sampleRate) override;
        void close() override;

        int progress() const;

        void pause();
        void resume();
        void cancel();

        enum Status {
            Running,
            Paused,
            Cancelled,
            Ready,
        };
        Status status() const;

        void wait();
        PositionableAudioSource *source() const;

    signals:
        void statusChanged(Status status);
        void progressChanged(int progress);
    };

} // talcs

#endif // TALCS_FUTUREAUDIOSOURCE_H
