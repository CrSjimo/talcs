#ifndef TALCS_AUDIOSOURCECLIPSERIES_P_H
#define TALCS_AUDIOSOURCECLIPSERIES_P_H

#include "AudioSourceClipSeries.h"
#include "PositionableAudioSource_p.h"
#include <QMutex>

namespace talcs {

    template <class ClipClass, class SeriesClass>
    class AudioSourceClipSeriesImpl {
    public:
        explicit AudioSourceClipSeriesImpl(SeriesClass *q): q(q) {
        }

        bool open(qint64 bufferSize, double sampleRate) {
            return std::all_of(q->m_clips.begin(), q->m_clips.end(),
                        [=](const ClipClass &clip) { return clip.content()->open(bufferSize, sampleRate); });
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
