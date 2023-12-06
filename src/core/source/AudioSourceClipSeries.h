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

#ifndef AUDIOSOURCECLIPSERIES_H
#define AUDIOSOURCECLIPSERIES_H

#include <TalcsCore/PositionableAudioSource.h>
#include <TalcsCore/AudioClipBase.h>

namespace talcs {

    using AudioSourceClip = AudioClipBase<PositionableAudioSource>;

    class AudioSourceClipSeriesPrivate;

    template <class ClipClass, class SeriesClass>
    class AudioSourceClipSeriesImpl;

    class TALCSCORE_EXPORT AudioSourceClipSeries
        : public PositionableAudioSource,
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

#endif // AUDIOSOURCECLIPSERIES_H
