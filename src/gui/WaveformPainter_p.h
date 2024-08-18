/******************************************************************************
 * Copyright (c) 2024 CrSjimo                                                 *
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

#ifndef TALCS_WAVEFORMPAINTER_P_H
#define TALCS_WAVEFORMPAINTER_P_H

#include <TalcsGui/WaveformPainter.h>

#include <QThread>

#include <TalcsCore/AudioBuffer.h>

namespace talcs {

    class WaveformPainterWorker : public QObject {
        Q_OBJECT
    public:
        void work(qint64 startPos, qint64 length);

        WaveformPainterPrivate *d;

    signals:
        void workRequired(qint64 startPos, qint64 length);
    };

    class WaveformPainterPrivate {
        Q_DECLARE_PUBLIC(WaveformPainter);
        friend class WaveformPainterWorker;
    public:
        WaveformPainter *q_ptr;

        PositionableAudioSource *src{};
        int channelCount{};
        bool mergeChannels{};
        qint64 length{};

        QVector<QVector<QPair<qint8, qint8>>> mipmap16;
        QVector<QVector<QPair<qint8, qint8>>> mipmap256;
        QVector<QVector<QPair<qint8, qint8>>> mipmap4096;

        QThread workerThread;
        WaveformPainterWorker worker;
        AudioBuffer buf;
        bool isInterrupted = true;
    };
}


#endif //TALCS_WAVEFORMPAINTER_P_H
