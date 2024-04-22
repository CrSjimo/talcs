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

#ifndef TALCS_AUDIOFORMATIO_P_H
#define TALCS_AUDIOFORMATIO_P_H

#include <QPointer>

#include <TalcsFormat/AudioFormatIO.h>

namespace talcs {

    class AudioFormatIOPrivate {
        Q_DECLARE_PUBLIC(AudioFormatIO)
    public:
        AudioFormatIO *q_ptr;
        QPointer<QIODevice> stream;
        QScopedPointer<SndfileHandle> sf;
        AbstractAudioFormatIO::OpenMode openMode = AbstractAudioFormatIO::NotOpen;

        double compressionLevel = 0.0;

        int64_t sfVioGetFilelen() const;
        int64_t sfVioSeek(int64_t offset, int whence) const;
        int64_t sfVioRead(void *ptr, int64_t count) const;
        int64_t sfVioWrite(const void *ptr, int64_t count) const;
        int64_t sfVioTell() const;
    };
    
}

#endif // TALCS_AUDIOFORMATIO_P_H
