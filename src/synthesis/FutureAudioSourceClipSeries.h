#ifndef TALCS_FUTUREAUDIOSOURCECLIPSERIES_H
#define TALCS_FUTUREAUDIOSOURCECLIPSERIES_H

#include "source/PositionableAudioSource.h"
#include "utils/AudioClipBase.h"
#include <QObject>

namespace talcs {

    class FutureAudioSource;
    using FutureAudioSourceClip = AudioClipBase<FutureAudioSource>;

    class FutureAudioSourceClipSeriesPrivate;

    class TALCS_EXPORT FutureAudioSourceClipSeries : public QObject,
                                        public PositionableAudioSource,
                                        public AudioClipSeriesBase<FutureAudioSource> {
        Q_OBJECT
        Q_DECLARE_PRIVATE_D(PositionableAudioSource::d_ptr, FutureAudioSourceClipSeries)
    public:
        explicit FutureAudioSourceClipSeries(QObject *parent = nullptr);
        ~FutureAudioSourceClipSeries() override;

        qint64 read(const AudioSourceReadData &readData) override;
        qint64 length() const override;
        qint64 nextReadPosition() const override;
        void setNextReadPosition(qint64 pos) override;
        bool addClip(const AudioClipBase<FutureAudioSource> &clip) override;
        bool removeClipAt(qint64 pos) override;
        void clearClips() override;
        bool open(qint64 bufferSize, double sampleRate) override;
        void close() override;

        qint64 lengthAvailableInTotal() const;
        qint64 lengthAvailableFrom(qint64 pos) const;

        enum ReadMode {
            Notify,
            Skip,
            Block,
        };
        void setReadMode(ReadMode readMode);
        ReadMode readMode() const;
        bool isCurrentPositionReady() const;

    signals:
        void progressChanged(qint64 lengthAvailableInTotal, qint64 effectiveLength);
        void pauseRequired();
        void resumeRequired();
    };

} // talcs

#endif // TALCS_FUTUREAUDIOSOURCECLIPSERIES_H
