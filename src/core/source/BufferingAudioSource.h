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

#ifndef TALCS_BUFFERINGAUDIOSOURCE_H
#define TALCS_BUFFERINGAUDIOSOURCE_H

#include <TalcsCore/PositionableAudioSource.h>
#include <QDeadlineTimer>

class QThreadPool;

namespace talcs {

    class BufferingAudioSourcePrivate;

    class TALCSCORE_EXPORT BufferingAudioSource : public PositionableAudioSource {
        Q_DECLARE_PRIVATE(BufferingAudioSource)
    public:
        explicit BufferingAudioSource(PositionableAudioSource *src, int channelCount, qint64 readAheadSize,
                                      bool autoBuffering = true, QThreadPool *threadPool = nullptr);
        explicit BufferingAudioSource(PositionableAudioSource *src, bool takeOwnership, int channelCount,
                                      qint64 readAheadSize, bool autoBuffering = true, QThreadPool *threadPool = nullptr);

        ~BufferingAudioSource() override;

        qint64 read(const AudioSourceReadData &readData) override;
        qint64 length() const override;
        void setNextReadPosition(qint64 pos) override;

        bool open(qint64 bufferSize, double sampleRate) override;
        void close() override;

        void setReadAheadSize(qint64 size);
        qint64 readAheadSize() const;

        void setChannelCount(int channelCount);
        int channelCount() const;

        void setSource(PositionableAudioSource *src, bool takeOwnership = false);
        PositionableAudioSource *source() const;

        bool waitForBuffering(QDeadlineTimer deadline = QDeadlineTimer::Forever);

        void flush();

        static QThreadPool *threadPool();

    protected:
        explicit BufferingAudioSource(BufferingAudioSourcePrivate &d);
    };

} // talcs

#endif //TALCS_BUFFERINGAUDIOSOURCE_H
