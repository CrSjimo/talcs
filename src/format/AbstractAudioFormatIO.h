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

#ifndef TALCS_ABSTRACTAUDIOFORMATIO_H
#define TALCS_ABSTRACTAUDIOFORMATIO_H

#include <QtGlobal>

namespace talcs {

    class AbstractAudioFormatIO {
    public:
        virtual ~AbstractAudioFormatIO() = default;

        enum OpenModeFlag {
            NotOpen = 0x00,
            Read = 0x01,
            Write = 0x02,
        };

        Q_DECLARE_FLAGS(OpenMode, OpenModeFlag)

        virtual bool open(OpenMode mode) = 0;
        virtual bool open(OpenMode mode, int format, int channels, double sampleRate) = 0;
        virtual OpenMode openMode() const = 0;
        virtual void close() = 0;

        virtual int channelCount() const = 0;
        virtual double sampleRate() const = 0;
        virtual qint64 length() const = 0;

        virtual qint64 read(float *ptr, qint64 length) = 0;
        virtual qint64 write(const float *ptr, qint64 length) = 0;
        virtual qint64 seek(qint64 pos) = 0;
        virtual qint64 pos() const = 0;
    };

} // talcs

#endif //TALCS_ABSTRACTAUDIOFORMATIO_H
