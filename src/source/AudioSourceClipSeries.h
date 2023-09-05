#ifndef AUDIOSOURCECLIPSERIES_H
#define AUDIOSOURCECLIPSERIES_H

#include "PositionableAudioSource.h"
#include "utils/AudioClipBase.h"

namespace talcs {
    typedef AudioClipBase<PositionableAudioSource> AudioSourceClip;

    class AudioSourceClipSeriesPrivate;

    class TALCS_EXPORT AudioSourceClipSeries : public PositionableAudioSource {
        Q_DECLARE_PRIVATE(AudioSourceClipSeries)
    public:
        AudioSourceClipSeries();
        ~AudioSourceClipSeries() override;
        qint64 read(const AudioSourceReadData &readData) override;
        qint64 length() const override;
        qint64 nextReadPosition() const override;
        void setNextReadPosition(qint64 pos) override;
        bool open(qint64 bufferSize, double sampleRate) override;
        void close() override;

        bool addClip(const AudioSourceClip &clip);
        virtual bool removeClipAt(qint64 pos);
        void clearClips();
        AudioSourceClip findClipAt(qint64 pos) const;
        QList<AudioSourceClip> clips() const;
        qint64 effectiveLength() const;

    protected:
        explicit AudioSourceClipSeries(AudioSourceClipSeriesPrivate &d);
    };
}

#endif // AUDIOSOURCECLIPSERIES_H
