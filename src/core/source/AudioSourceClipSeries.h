#ifndef TALCS_AUDIOSOURCECLIPSERIES_H
#define TALCS_AUDIOSOURCECLIPSERIES_H

#include "PositionableAudioSource.h"
#include "core/base/AudioClipBase.h"

namespace talcs {
    typedef AudioClipBase<PositionableAudioSource> AudioSourceClip;

    class AudioSourceClipSeriesPrivate;

    template <class ClipClass, class SeriesClass>
    class AudioSourceClipSeriesImpl;

    class TALCS_EXPORT AudioSourceClipSeries : public PositionableAudioSource,
                                               public AudioClipSeriesBase<PositionableAudioSource> {
        Q_DECLARE_PRIVATE(AudioSourceClipSeries)
        friend class AudioSourceClipSeriesImpl<AudioSourceClip, AudioSourceClipSeries>;
    public:
        AudioSourceClipSeries();
        ~AudioSourceClipSeries() override;
        qint64 read(const AudioSourceReadData &readData) override;
        qint64 length() const override;
        qint64 nextReadPosition() const override;
        void setNextReadPosition(qint64 pos) override;
        bool open(qint64 bufferSize, double sampleRate) override;
        void close() override;

        bool addClip(const AudioSourceClip &clip) override;
        bool removeClipAt(qint64 pos) override;
        void clearClips() override;

    protected:
        explicit AudioSourceClipSeries(AudioSourceClipSeriesPrivate &d);
    };
}

#endif // TALCS_AUDIOSOURCECLIPSERIES_H
