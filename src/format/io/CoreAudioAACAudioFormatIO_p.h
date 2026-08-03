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

#ifndef TALCS_COREAUDIOAACAUDIOFORMATIO_P_H
#define TALCS_COREAUDIOAACAUDIOFORMATIO_P_H

#include <QPointer>

#include <AudioToolbox/AudioToolbox.h>

#include <TalcsFormat/CoreAudioAACAudioFormatIO.h>

namespace talcs {

    class CoreAudioAACAudioFormatIOPrivate {
        Q_DECLARE_PUBLIC(CoreAudioAACAudioFormatIO)
    public:
        CoreAudioAACAudioFormatIO *q_ptr;

        QPointer<QIODevice> stream;
        qint64 streamOffset = 0;

        AudioFileID audioFile = nullptr;
        ExtAudioFileRef extAudioFile = nullptr;
        AbstractAudioFormatIO::OpenMode openMode = AbstractAudioFormatIO::NotOpen;
        UInt32 channelCount = 0;
        Float64 sampleRate = 0;
        UInt32 bytesPerFrame = 0;
        qint64 frameLength = 0;
        qint64 framePosition = 0;

        void releaseResources();

        static OSStatus readCallback(void *clientData, SInt64 position, UInt32 requestCount,
                                     void *buffer, UInt32 *actualCount);
        static SInt64 getSizeCallback(void *clientData);
    };

}

#endif // TALCS_COREAUDIOAACAUDIOFORMATIO_P_H
