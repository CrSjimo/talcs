#ifndef TALCS_AUDIOSOURCECLIPSERIES_P_H
#define TALCS_AUDIOSOURCECLIPSERIES_P_H

#include <QMutex>

#include <TalcsCore/AudioSourceClipSeries.h>
#include <TalcsCore/private/PositionableAudioSource_p.h>

namespace talcs {

    template <class ClipClass, class SeriesClass>
    class AudioSourceClipSeriesImpl {
    public:
        explicit AudioSourceClipSeriesImpl(SeriesClass *q): q(q) {
        }

        bool open(qint64 bufferSize, double sampleRate) {
            for (auto p = q->m_clips.begin(); p != q->m_clips.end(); p++) {
                auto clip = p->interval();
                if (!clip.content()->open(bufferSize, sampleRate))
                    return false;
            }
            return true;
        }

        void close() {
            std::for_each(q->m_clips.begin(), q->m_clips.end(),
                          [=](const ClipClass &clip) { return clip.content()->close(); });
        }

        bool addClip(const ClipClass &clip) {
            if (q->isOpen()) {
                if (!clip.content()->open(q->bufferSize(), q->sampleRate())) {
                    return false;
                }
            }
            return true;
        }

        void removeClip(const ClipClass &clip) {
            clip.content()->close();
        }

        void clearClips() {
            for (const auto &clip : q->m_clips)
                clip.content()->close();
        }

     private:
        SeriesClass *q;
    };

    class AudioSourceClipSeriesPrivate : public PositionableAudioSourcePrivate, public AudioSourceClipSeriesImpl<AudioSourceClip, AudioSourceClipSeries> {
        Q_DECLARE_PUBLIC(AudioSourceClipSeries);
    public:
        explicit AudioSourceClipSeriesPrivate(AudioSourceClipSeries *q);
        QMutex mutex;
    };
    
}

#endif // TALCS_AUDIOSOURCECLIPSERIES_P_H
