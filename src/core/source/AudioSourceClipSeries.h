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

#include <TalcsCore/IClipSeries.h>
#include <TalcsCore/PositionableAudioSource.h>

namespace talcs {

    class AudioSourceClipSeriesPrivate;

    class TALCSCORE_EXPORT AudioSourceClipSeries
        : public PositionableAudioSource,
          public IClipSeries<PositionableAudioSource> {
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

        ClipView insertClip(PositionableAudioSource *content, qint64 position, qint64 startPos, qint64 length) override;
        void setClipStartPos(const ClipView &clip, qint64 startPos) override;
        bool setClipRange(const ClipView &clip, qint64 position, qint64 length) override;
        ClipView setClipContent(const ClipView &clip, PositionableAudioSource *content) override;
        ClipView findClip(PositionableAudioSource *content) const override;
        QList<ClipView> findClip(qint64 position) const override;
        void removeClip(const ClipView &clip) override;
        void removeAllClips() override;
        QList<ClipView> clips() const override;
        qint64 effectiveLength() const override;

    protected:
        explicit AudioSourceClipSeries(AudioSourceClipSeriesPrivate &d);
    };

}

#endif // AUDIOSOURCECLIPSERIES_H
