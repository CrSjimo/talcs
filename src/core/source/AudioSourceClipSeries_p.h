/******************************************************************************
 * Copyright (c) 2023 CrSjimo                                                 *
 *                                                                            *
 * This file is part of TALCS.                                                *
 *                                                                            *
 * TALCS is free software: you can redistribute it and/or modify it under the *
 * terms of the GNU Lesser General Public License as published by the Free    *
 * Software Foundation, either version 3 of the License, or (at your option)  *
 * any later version.                                                         *
 *                                                                            *
 * TALCS is distributed in the hope that it will be useful, but WITHOUT ANY   *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS  *
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for    *
 * more details.                                                              *
 *                                                                            *
 * You should have received a copy of the GNU Lesser General Public License   *
 * along with TALCS. If not, see <https://www.gnu.org/licenses/>.             *
 ******************************************************************************/

#ifndef TALCS_AUDIOSOURCECLIPSERIES_P_H
#define TALCS_AUDIOSOURCECLIPSERIES_P_H

#include <algorithm>

#include <QMutex>

#include <TalcsCore/AudioBuffer.h>
#include <TalcsCore/AudioSourceClipSeries.h>
#include <TalcsCore/private/IClipSeries_p.h>
#include <TalcsCore/private/PositionableAudioSource_p.h>

namespace talcs {

    template <class SourceClass, class SeriesClassPrivate>
    class AudioSourceClipSeriesBase {
    public:
        explicit AudioSourceClipSeriesBase(SeriesClassPrivate *d): d(d) {
        }

        bool openAllClips(qint64 bufferSize, double sampleRate) {
            buf.resize(2, bufferSize);
            for (auto p = d->clips.begin(); p != d->clips.end(); p++) {
                if (!reinterpret_cast<SourceClass *>(p->interval().content())->open(bufferSize, sampleRate))
                    return false;
            }
            return true;
        }

        void closeAllClips() {
            buf.resize(0, 0);
            for (auto p = d->clips.begin(); p != d->clips.end(); p++) {
                reinterpret_cast<SourceClass *>(p->interval().content())->close();
            }
        }

        bool preInsertClip(SourceClass *src) {
            if (d->q_ptr->isOpen()) {
                if (!src->open(d->q_ptr->bufferSize(), d->q_ptr->sampleRate())) {
                    return false;
                }
            }
            return true;
        }

        QPair<qint64, AudioSourceReadData> calculateClipReadData(const IClipSeriesPrivate::ClipInterval &clip, qint64 seriesPosition,
                                                                        const AudioSourceReadData &seriesReadData) {
            auto headCut = qMax(0ll, seriesPosition - clip.position());
            auto tailCut = qMax(0ll, (clip.position() + clip.length()) - (seriesPosition + seriesReadData.length));
            auto readStart = qMax(0ll, clip.position() - seriesPosition);
            qint64 clipReadPosition = headCut + d->clipStartPosDict.value(clip.content());
            buf.clear();
            if (buf.channelCount() < seriesReadData.buffer->channelCount())
                buf.resize(seriesReadData.buffer->channelCount(), -1);
            return {clipReadPosition, {
                    &buf,
                    readStart + seriesReadData.startPos,
                    clip.length() - headCut - tailCut,
                    seriesReadData.silentFlags,
            }};
        }

        AudioBuffer buf;

    private:
        SeriesClassPrivate *d;
    };

    class AudioSourceClipSeriesPrivate : public PositionableAudioSourcePrivate, public IClipSeriesPrivate, public AudioSourceClipSeriesBase<PositionableAudioSource, AudioSourceClipSeriesPrivate> {
        Q_DECLARE_PUBLIC(AudioSourceClipSeries);
    public:
        AudioSourceClipSeriesPrivate();
        QMutex mutex;
    };
    
}

#endif // TALCS_AUDIOSOURCECLIPSERIES_P_H
