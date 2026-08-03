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

#ifndef TALCS_MEDIAFOUNDATIONAACAUDIOFORMATIO_P_H
#define TALCS_MEDIAFOUNDATIONAACAUDIOFORMATIO_P_H

#ifndef NOMINMAX
#  define NOMINMAX
#endif

#include <QByteArray>
#include <QPointer>
#include <QString>

#include <mfidl.h>
#include <mfreadwrite.h>
#include <wrl/client.h>

#include <TalcsFormat/MediaFoundationAACAudioFormatIO.h>

namespace talcs {

    class MediaFoundationAACAudioFormatIOPrivate {
        Q_DECLARE_PUBLIC(MediaFoundationAACAudioFormatIO)
    public:
        MediaFoundationAACAudioFormatIO *q_ptr;

        QPointer<QIODevice> stream;
        qint64 streamOffset = 0;

        Microsoft::WRL::ComPtr<IStream> inputStream;
        Microsoft::WRL::ComPtr<IMFByteStream> byteStream;
        Microsoft::WRL::ComPtr<IMFSourceReader> sourceReader;

        AbstractAudioFormatIO::OpenMode openMode = AbstractAudioFormatIO::NotOpen;
        DWORD audioStreamIndex = MF_SOURCE_READER_FIRST_AUDIO_STREAM;
        UINT32 channelCount = 0;
        UINT32 sampleRate = 0;
        UINT32 blockAlignment = 0;
        qint64 frameLength = 0;
        qint64 framePosition = 0;

        QByteArray decodedData;
        qint64 decodedStartFrame = 0;
        qint64 decodedFrameOffset = 0;
        qint64 decodedFrameCount = 0;
        bool endOfStream = false;

        bool comInitialized = false;
        bool mediaFoundationStarted = false;

        void clearDecodedData();
        void releaseResources();
        HRESULT fillDecodedData(QString &error);
    };

}

#endif // TALCS_MEDIAFOUNDATIONAACAUDIOFORMATIO_P_H
