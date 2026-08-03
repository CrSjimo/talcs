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

#include "CoreAudioAACAudioFormatIO.h"
#include "CoreAudioAACAudioFormatIO_p.h"

#include <algorithm>
#include <limits>

#include <QDebug>
#include <QIODevice>

#define TEST_IS_OPEN(ret)                                                                                              \
    if (!d->extAudioFile || !d->openMode.testFlag(Read)) {                                                             \
        qWarning() << "CoreAudioAACAudioFormatIO: Not open.";                                                         \
        return ret;                                                                                                    \
    }

namespace talcs {

    namespace {

        QString makeOSStatusError(const QString &operation, OSStatus status) {
            return QStringLiteral("%1 (OSStatus %2)").arg(operation).arg(status);
        }

        bool isAACFormat(AudioFormatID format) {
            switch (format) {
                case kAudioFormatMPEG4AAC:
                case kAudioFormatMPEG4AAC_HE:
                case kAudioFormatMPEG4AAC_HE_V2:
                case kAudioFormatMPEG4AAC_LD:
                case kAudioFormatMPEG4AAC_ELD:
                case kAudioFormatMPEG4AAC_ELD_SBR:
                case kAudioFormatMPEG4AAC_ELD_V2:
                case kAudioFormatMPEG4AAC_Spatial:
                    return true;
                default:
                    return false;
            }
        }

    }

    void CoreAudioAACAudioFormatIOPrivate::releaseResources() {
        if (extAudioFile) {
            ExtAudioFileDispose(extAudioFile);
            extAudioFile = nullptr;
            audioFile = nullptr;
        } else if (audioFile) {
            AudioFileClose(audioFile);
            audioFile = nullptr;
        }

        openMode = AbstractAudioFormatIO::NotOpen;
        channelCount = 0;
        sampleRate = 0;
        bytesPerFrame = 0;
        frameLength = 0;
        framePosition = 0;
    }

    OSStatus CoreAudioAACAudioFormatIOPrivate::readCallback(void *clientData, SInt64 position,
                                                            UInt32 requestCount, void *buffer,
                                                            UInt32 *actualCount) {
        auto *d = static_cast<CoreAudioAACAudioFormatIOPrivate *>(clientData);
        if (actualCount)
            *actualCount = 0;
        if (!d->stream)
            return kAudioFileNotOpenError;
        if (position < 0 || position > (std::numeric_limits<qint64>::max)() - d->streamOffset)
            return kAudioFilePositionError;
        if (!d->stream->seek(d->streamOffset + position))
            return kAudioFilePositionError;

        const auto result = d->stream->read(static_cast<char *>(buffer), requestCount);
        if (result < 0)
            return kAudioFileUnspecifiedError;
        if (actualCount)
            *actualCount = static_cast<UInt32>(result);
        return noErr;
    }

    SInt64 CoreAudioAACAudioFormatIOPrivate::getSizeCallback(void *clientData) {
        const auto *d = static_cast<const CoreAudioAACAudioFormatIOPrivate *>(clientData);
        if (!d->stream)
            return 0;
        return (std::max)(qint64(0), d->stream->size() - d->streamOffset);
    }

    CoreAudioAACAudioFormatIO::CoreAudioAACAudioFormatIO(QIODevice *stream)
        : d_ptr(new CoreAudioAACAudioFormatIOPrivate) {
        Q_D(CoreAudioAACAudioFormatIO);
        d->q_ptr = this;
        setStream(stream);
    }

    CoreAudioAACAudioFormatIO::~CoreAudioAACAudioFormatIO() {
        CoreAudioAACAudioFormatIO::close();
    }

    void CoreAudioAACAudioFormatIO::setStream(QIODevice *stream, qint64 offset) {
        Q_D(CoreAudioAACAudioFormatIO);
        if (d->openMode) {
            qWarning() << "CoreAudioAACAudioFormatIO: Cannot set stream while the decoder is open.";
            return;
        }
        d->stream = stream;
        d->streamOffset = offset;
    }

    QIODevice *CoreAudioAACAudioFormatIO::stream() const {
        Q_D(const CoreAudioAACAudioFormatIO);
        return d->stream;
    }

    bool CoreAudioAACAudioFormatIO::open(OpenMode mode) {
        Q_D(CoreAudioAACAudioFormatIO);
        close();

        const auto fail = [this, d](const QString &error) {
            d->releaseResources();
            setErrorString(error);
            qWarning().noquote() << "CoreAudioAACAudioFormatIO:" << error;
            return false;
        };
        const auto failOSStatus = [&fail](const QString &operation, OSStatus status) {
            return fail(makeOSStatusError(operation, status));
        };

        if (mode.testFlag(Write))
            return fail(QStringLiteral("Writing is not supported"));
        if (!mode.testFlag(Read))
            return fail(QStringLiteral("Cannot open because read access is not specified"));
        if (!d->stream)
            return fail(QStringLiteral("Cannot open because the stream is null"));

        auto status = AudioFileOpenWithCallbacks(d, CoreAudioAACAudioFormatIOPrivate::readCallback, nullptr,
                                                 CoreAudioAACAudioFormatIOPrivate::getSizeCallback, nullptr, 0,
                                                 &d->audioFile);
        if (status != noErr)
            return failOSStatus(QStringLiteral("Failed to parse the input audio file"), status);

        AudioFileTypeID fileType = 0;
        UInt32 propertySize = sizeof(fileType);
        status = AudioFileGetProperty(d->audioFile, kAudioFilePropertyFileFormat, &propertySize, &fileType);
        if (status != noErr)
            return failOSStatus(QStringLiteral("Failed to get the audio file container type"), status);
        if (fileType != kAudioFileMPEG4Type && fileType != kAudioFileM4AType)
            return fail(QStringLiteral("The input is not an MPEG-4 or M4A audio file"));

        UInt32 audioTrackCount = 0;
        propertySize = sizeof(audioTrackCount);
        status = AudioFileGetProperty(d->audioFile, kAudioFilePropertyAudioTrackCount,
                                      &propertySize, &audioTrackCount);
        if (status != noErr)
            return failOSStatus(QStringLiteral("Failed to get the MPEG-4 audio track count"), status);

        bool foundAACTrack = false;
        AudioStreamBasicDescription fileFormat{};
        for (UInt32 trackIndex = 0; trackIndex < audioTrackCount; ++trackIndex) {
            status = AudioFileSetProperty(d->audioFile, kAudioFilePropertyUseAudioTrack,
                                          sizeof(trackIndex), &trackIndex);
            if (status != noErr)
                return failOSStatus(QStringLiteral("Failed to select an MPEG-4 audio track"), status);

            propertySize = sizeof(fileFormat);
            status = AudioFileGetProperty(d->audioFile, kAudioFilePropertyDataFormat,
                                          &propertySize, &fileFormat);
            if (status != noErr)
                return failOSStatus(QStringLiteral("Failed to get an MPEG-4 audio track format"), status);
            if (isAACFormat(fileFormat.mFormatID)) {
                foundAACTrack = true;
                break;
            }
        }
        if (!foundAACTrack)
            return fail(QStringLiteral("The MPEG-4 file does not contain AAC audio"));
        if (fileFormat.mSampleRate <= 0 || fileFormat.mChannelsPerFrame == 0)
            return fail(QStringLiteral("The AAC audio stream has an invalid format"));

        status = ExtAudioFileWrapAudioFileID(d->audioFile, false, &d->extAudioFile);
        if (status != noErr)
            return failOSStatus(QStringLiteral("Failed to create the Core Audio decoder"), status);

        AudioStreamBasicDescription clientFormat{};
        clientFormat.mSampleRate = fileFormat.mSampleRate;
        clientFormat.mFormatID = kAudioFormatLinearPCM;
        clientFormat.mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked |
                                    kAudioFormatFlagsNativeEndian;
        clientFormat.mBytesPerPacket = fileFormat.mChannelsPerFrame * sizeof(float);
        clientFormat.mFramesPerPacket = 1;
        clientFormat.mBytesPerFrame = clientFormat.mBytesPerPacket;
        clientFormat.mChannelsPerFrame = fileFormat.mChannelsPerFrame;
        clientFormat.mBitsPerChannel = 32;

        status = ExtAudioFileSetProperty(d->extAudioFile, kExtAudioFileProperty_ClientDataFormat,
                                         sizeof(clientFormat), &clientFormat);
        if (status != noErr)
            return failOSStatus(QStringLiteral("Failed to configure floating-point AAC output"), status);

        SInt64 frameLength = 0;
        propertySize = sizeof(frameLength);
        status = ExtAudioFileGetProperty(d->extAudioFile, kExtAudioFileProperty_FileLengthFrames,
                                         &propertySize, &frameLength);
        if (status != noErr)
            return failOSStatus(QStringLiteral("Failed to get the AAC audio length"), status);
        if (frameLength < 0)
            return fail(QStringLiteral("The AAC audio stream returned an invalid length"));

        d->channelCount = clientFormat.mChannelsPerFrame;
        d->sampleRate = clientFormat.mSampleRate;
        d->bytesPerFrame = clientFormat.mBytesPerFrame;
        d->frameLength = frameLength;
        d->framePosition = 0;
        d->openMode = Read;
        clearErrorString();
        return true;
    }

    AbstractAudioFormatIO::OpenMode CoreAudioAACAudioFormatIO::openMode() const {
        Q_D(const CoreAudioAACAudioFormatIO);
        return d->openMode;
    }

    void CoreAudioAACAudioFormatIO::close() {
        Q_D(CoreAudioAACAudioFormatIO);
        d->releaseResources();
        clearErrorString();
    }

    int CoreAudioAACAudioFormatIO::format() const {
        Q_D(const CoreAudioAACAudioFormatIO);
        TEST_IS_OPEN(0)
        return AAC;
    }

    void CoreAudioAACAudioFormatIO::setFormat(int) {
        qWarning() << "CoreAudioAACAudioFormatIO: Writing is not supported.";
    }

    int CoreAudioAACAudioFormatIO::channelCount() const {
        Q_D(const CoreAudioAACAudioFormatIO);
        TEST_IS_OPEN(0)
        return static_cast<int>(d->channelCount);
    }

    void CoreAudioAACAudioFormatIO::setChannelCount(int) {
        qWarning() << "CoreAudioAACAudioFormatIO: Writing is not supported.";
    }

    double CoreAudioAACAudioFormatIO::sampleRate() const {
        Q_D(const CoreAudioAACAudioFormatIO);
        TEST_IS_OPEN(0.0)
        return d->sampleRate;
    }

    void CoreAudioAACAudioFormatIO::setSampleRate(double) {
        qWarning() << "CoreAudioAACAudioFormatIO: Writing is not supported.";
    }

    qint64 CoreAudioAACAudioFormatIO::length() const {
        Q_D(const CoreAudioAACAudioFormatIO);
        TEST_IS_OPEN(0)
        return d->frameLength;
    }

    qint64 CoreAudioAACAudioFormatIO::read(float *ptr, qint64 length) {
        Q_D(CoreAudioAACAudioFormatIO);
        TEST_IS_OPEN(0)
        if (length <= 0)
            return 0;
        if (!ptr) {
            const auto error = QStringLiteral("Cannot read into a null buffer");
            setErrorString(error);
            qWarning().noquote() << "CoreAudioAACAudioFormatIO:" << error;
            return 0;
        }
        if (d->framePosition >= d->frameLength)
            return 0;

        const auto requestedFrames = (std::min)(length, d->frameLength - d->framePosition);
        const auto maximumFramesPerRead = static_cast<qint64>((std::numeric_limits<UInt32>::max)() /
                                                               d->bytesPerFrame);
        qint64 framesRead = 0;
        while (framesRead < requestedFrames) {
            auto currentFrames = static_cast<UInt32>((std::min)(requestedFrames - framesRead,
                                                                 maximumFramesPerRead));
            AudioBufferList bufferList{};
            bufferList.mNumberBuffers = 1;
            bufferList.mBuffers[0].mNumberChannels = d->channelCount;
            bufferList.mBuffers[0].mDataByteSize = currentFrames * d->bytesPerFrame;
            bufferList.mBuffers[0].mData = ptr + framesRead * d->channelCount;

            const auto status = ExtAudioFileRead(d->extAudioFile, &currentFrames, &bufferList);
            if (status != noErr) {
                const auto error = makeOSStatusError(QStringLiteral("Failed to decode AAC audio"), status);
                setErrorString(error);
                qWarning().noquote() << "CoreAudioAACAudioFormatIO:" << error;
                break;
            }
            if (currentFrames == 0)
                break;
            framesRead += currentFrames;
            d->framePosition += currentFrames;
        }
        return framesRead;
    }

    qint64 CoreAudioAACAudioFormatIO::write(const float *, qint64) {
        qWarning() << "CoreAudioAACAudioFormatIO: Writing is not supported.";
        return 0;
    }

    qint64 CoreAudioAACAudioFormatIO::seek(qint64 pos) {
        Q_D(CoreAudioAACAudioFormatIO);
        TEST_IS_OPEN(-1)
        if (pos < 0 || pos > d->frameLength) {
            const auto error = QStringLiteral("The requested audio position is outside the stream");
            setErrorString(error);
            qWarning().noquote() << "CoreAudioAACAudioFormatIO:" << error;
            return -1;
        }
        if (pos == d->framePosition) {
            clearErrorString();
            return pos;
        }

        const auto status = ExtAudioFileSeek(d->extAudioFile, pos);
        if (status != noErr) {
            const auto error = makeOSStatusError(QStringLiteral("Failed to seek the AAC audio stream"), status);
            setErrorString(error);
            qWarning().noquote() << "CoreAudioAACAudioFormatIO:" << error;
            return -1;
        }
        d->framePosition = pos;
        clearErrorString();
        return pos;
    }

    qint64 CoreAudioAACAudioFormatIO::pos() const {
        Q_D(const CoreAudioAACAudioFormatIO);
        TEST_IS_OPEN(0)
        return d->framePosition;
    }

}
