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

#ifndef TALCS_FUTUREAUDIOSOURCECLIPSERIES_H
#define TALCS_FUTUREAUDIOSOURCECLIPSERIES_H

#include <QObject>

#include <TalcsCore/PositionableAudioSource.h>
#include <TalcsCore/IClipSeries.h>
#include <TalcsSynthesis/TalcsSynthesisGlobal.h>

namespace talcs {

    class FutureAudioSource;

    class FutureAudioSourceClipSeriesPrivate;

    class TransportAudioSource;

    class TALCSSYNTHESIS_EXPORT FutureAudioSourceClipSeries
        : public QObject,
          public PositionableAudioSource,
          public IClipSeries<FutureAudioSource> {
        Q_OBJECT
        Q_DECLARE_PRIVATE_D(PositionableAudioSource::d_ptr, FutureAudioSourceClipSeries)
    public:
        explicit FutureAudioSourceClipSeries(QObject *parent = nullptr);
        ~FutureAudioSourceClipSeries() override;

        qint64 read(const AudioSourceReadData &readData) override;
        qint64 length() const override;
        qint64 nextReadPosition() const override;
        void setNextReadPosition(qint64 pos) override;
        bool open(qint64 bufferSize, double sampleRate) override;
        void close() override;

        ClipView insertClip(FutureAudioSource *content, qint64 position, qint64 startPos, qint64 length) override;
        void setClipStartPos(const ClipView &clip, qint64 startPos) override;
        bool setClipRange(const ClipView &clip, qint64 position, qint64 length) override;
        ClipView setClipContent(const ClipView &clip, FutureAudioSource *content) override;
        ClipView findClip(FutureAudioSource *content) const override;
        ClipView findClip(qint64 position) const override;

        void removeClip(const ClipView &clip) override;
        void removeAllClips() override;
        QList<ClipView> clips() const override;
        qint64 effectiveLength() const override;
        
        qint64 lengthAvailable() const;
        qint64 lengthLoaded() const;
        qint64 lengthOfAllClips() const;

        bool canRead(qint64 from, qint64 length) const;

        enum ReadMode {
            Notify,
            Skip,
            Block,
        };
        void setReadMode(ReadMode readMode);
        ReadMode readMode() const;
        void setBufferingTarget(TransportAudioSource *target);
        TransportAudioSource *bufferingTarget() const;

    signals:
        void progressChanged(qint64 lengthAvailable, qint64 lengthLoaded, qint64 lengthOfAllClips,
                             qint64 effectiveLength);
    };

}

#endif // TALCS_FUTUREAUDIOSOURCECLIPSERIES_H
