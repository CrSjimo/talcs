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

#ifndef TALCS_WAVPACKAUDIOFORMATIO_H
#define TALCS_WAVPACKAUDIOFORMATIO_H

#include <QScopedPointer>

#include <TalcsCore/ErrorStringProvider.h>
#include <TalcsFormat/AbstractAudioFormatIO.h>

class QFileDevice;

namespace talcs {

    class WavpackAudioFormatIOPrivate;

    class TALCSFORMAT_EXPORT WavpackAudioFormatIO : public AbstractAudioFormatIO, public ErrorStringProvider {
        Q_DECLARE_PRIVATE(WavpackAudioFormatIO)
    public:
        explicit WavpackAudioFormatIO(QFileDevice *stream = nullptr, QFileDevice *corrStream = nullptr);
        ~WavpackAudioFormatIO() override;

        void setStream(QFileDevice *stream, QFileDevice *corrStream = nullptr);
        QFileDevice *stream() const;
        QFileDevice *corrStream() const;

        bool open(OpenMode mode) override;
        OpenMode openMode() const override;
        void close() override;

        enum WavPackFormat {
            Unknown = 0x0000,
            Float = 0x0001,
            IntMask = 0x00FF,
        };

        int format() const override;
        void setFormat(int format) override;

        int channelCount() const override;
        void setChannelCount(int channelCount) override;

        double sampleRate() const override;
        void setSampleRate(double sampleRate) override;

        double bitRate() const;
        void setBitRate(double bitRate);

        void setThreadCount(int threadCount);
        int threadCount() const;

        qint64 length() const override;

        qint64 read(float *ptr, qint64 length) override;
        qint64 write(const float *ptr, qint64 length) override;

        qint64 seek(qint64 pos) override;
        qint64 pos() const override;

    private:
        QScopedPointer<WavpackAudioFormatIOPrivate> d_ptr;
    };

}

#endif //TALCS_WAVPACKAUDIOFORMATIO_H
