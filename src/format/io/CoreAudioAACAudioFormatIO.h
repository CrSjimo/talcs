/******************************************************************************
 * Copyright (c) 2026 CrSjimo                                                 *
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

#ifndef TALCS_COREAUDIOAACAUDIOFORMATIO_H
#define TALCS_COREAUDIOAACAUDIOFORMATIO_H

#include <QScopedPointer>

#include <TalcsCore/ErrorStringProvider.h>
#include <TalcsFormat/AbstractAudioFormatIO.h>
#include <TalcsFormat/TalcsFormatGlobal.h>

class QIODevice;

namespace talcs {

    class CoreAudioAACAudioFormatIOPrivate;

    class TALCSFORMAT_EXPORT CoreAudioAACAudioFormatIO : public AbstractAudioFormatIO,
                                                         public ErrorStringProvider {
        Q_DECLARE_PRIVATE(CoreAudioAACAudioFormatIO)
    public:
        explicit CoreAudioAACAudioFormatIO(QIODevice *stream = nullptr);
        ~CoreAudioAACAudioFormatIO() override;

        void setStream(QIODevice *stream, qint64 offset = 0);
        QIODevice *stream() const;

        bool open(OpenMode mode) override;
        OpenMode openMode() const override;
        void close() override;

        enum AACFormat {
            AAC = 0x0001,
        };

        int format() const override;
        void setFormat(int format) override;

        int channelCount() const override;
        void setChannelCount(int channelCount) override;

        double sampleRate() const override;
        void setSampleRate(double sampleRate) override;

        qint64 length() const override;

        qint64 read(float *ptr, qint64 length) override;
        qint64 write(const float *ptr, qint64 length) override;

        qint64 seek(qint64 pos) override;
        qint64 pos() const override;

    private:
        QScopedPointer<CoreAudioAACAudioFormatIOPrivate> d_ptr;
    };

}

#endif // TALCS_COREAUDIOAACAUDIOFORMATIO_H
