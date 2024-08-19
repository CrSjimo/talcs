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

#ifndef TALCS_WAVEFORMPAINTER_H
#define TALCS_WAVEFORMPAINTER_H

#include <QObject>

#include <TalcsGui/TalcsGuiGlobal.h>

class QPainter;

namespace talcs {

    class PositionableAudioSource;

    class WaveformPainterPrivate;

    class TALCSGUI_EXPORT WaveformPainter : public QObject {
        Q_OBJECT
        Q_DECLARE_PRIVATE(WaveformPainter)
    public:
        explicit WaveformPainter(QObject *parent = nullptr);
        ~WaveformPainter() override;

        void setSource(PositionableAudioSource *src, int channelCount, qint64 length, bool mergeChannels = false);

        void startLoad(qint64 startPosHint, qint64 length);
        void interruptLoad();

        QPair<qint8, qint8> getMinMaxFromMipmap(double startPosSecond, double lengthSecond, int channel = 0) const;

        void paint(QPainter *painter, const QRect &rect, double startPosSecond, double lengthSecond, int channel = 0, double verticalScale = 1.0) const;

    signals:
        void loadFinished(qint64 startPos, qint64 length);

    private:
        QScopedPointer<WaveformPainterPrivate> d_ptr;
    };

} // talcs

#endif //TALCS_WAVEFORMPAINTER_H
