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

#ifndef NOMINMAX
#  define NOMINMAX
#endif

#include "MediaFoundationAACAudioFormatIO.h"
#include "MediaFoundationAACAudioFormatIO_p.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstring>
#include <limits>
#include <new>

#include <QDebug>
#include <QIODevice>
#include <QMutex>
#include <QMutexLocker>
#include <QSharedPointer>

#include <mfapi.h>
#include <mferror.h>
#include <propidl.h>

#define TEST_IS_OPEN(ret)                                                                                              \
    if (!d->sourceReader || !d->openMode.testFlag(Read)) {                                                             \
        qWarning() << "MediaFoundationAACAudioFormatIO: Not open.";                                                   \
        return ret;                                                                                                    \
    }

namespace talcs {

    namespace {

        constexpr LONGLONG MediaFoundationTimeScale = 10000000;

        class QIODeviceIStream final : public IStream {
        public:
            QIODeviceIStream(QIODevice *stream, qint64 offset)
                : m_stream(stream), m_offset(offset), m_mutex(new QMutex) {
            }

            HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **object) override {
                if (!object)
                    return E_POINTER;
                *object = nullptr;
                if (riid == IID_IUnknown || riid == IID_ISequentialStream || riid == IID_IStream) {
                    *object = static_cast<IStream *>(this);
                    AddRef();
                    return S_OK;
                }
                return E_NOINTERFACE;
            }

            ULONG STDMETHODCALLTYPE AddRef() override {
                return ++m_referenceCount;
            }

            ULONG STDMETHODCALLTYPE Release() override {
                const auto count = --m_referenceCount;
                if (count == 0)
                    delete this;
                return count;
            }

            HRESULT STDMETHODCALLTYPE Read(void *data, ULONG byteCount, ULONG *bytesRead) override {
                if (bytesRead)
                    *bytesRead = 0;
                if (!data && byteCount != 0)
                    return STG_E_INVALIDPOINTER;

                QMutexLocker locker(m_mutex.data());
                auto *stream = m_stream.data();
                if (!stream)
                    return STG_E_REVERTED;
                if (m_position > (std::numeric_limits<qint64>::max)() - m_offset)
                    return STG_E_SEEKERROR;
                if (!stream->seek(m_offset + m_position))
                    return STG_E_SEEKERROR;

                const auto result = stream->read(static_cast<char *>(data), static_cast<qint64>(byteCount));
                if (result < 0)
                    return STG_E_READFAULT;
                m_position += result;
                if (bytesRead)
                    *bytesRead = static_cast<ULONG>(result);
                return result == byteCount ? S_OK : S_FALSE;
            }

            HRESULT STDMETHODCALLTYPE Write(const void *, ULONG, ULONG *bytesWritten) override {
                if (bytesWritten)
                    *bytesWritten = 0;
                return STG_E_ACCESSDENIED;
            }

            HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER move, DWORD origin, ULARGE_INTEGER *newPosition) override {
                QMutexLocker locker(m_mutex.data());
                auto *stream = m_stream.data();
                if (!stream)
                    return STG_E_REVERTED;

                qint64 base = 0;
                switch (origin) {
                    case STREAM_SEEK_SET:
                        break;
                    case STREAM_SEEK_CUR:
                        base = m_position;
                        break;
                    case STREAM_SEEK_END:
                        base = (std::max)(qint64(0), stream->size() - m_offset);
                        break;
                    default:
                        return STG_E_INVALIDFUNCTION;
                }

                const auto delta = move.QuadPart;
                if ((delta > 0 && base > (std::numeric_limits<qint64>::max)() - delta) ||
                    (delta < 0 && base < (std::numeric_limits<qint64>::min)() - delta)) {
                    return STG_E_INVALIDFUNCTION;
                }
                const auto position = base + delta;
                if (position < 0 || position > (std::numeric_limits<qint64>::max)() - m_offset)
                    return STG_E_INVALIDFUNCTION;
                if (!stream->seek(m_offset + position))
                    return STG_E_SEEKERROR;

                m_position = position;
                if (newPosition)
                    newPosition->QuadPart = static_cast<ULONGLONG>(position);
                return S_OK;
            }

            HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER) override {
                return STG_E_ACCESSDENIED;
            }

            HRESULT STDMETHODCALLTYPE CopyTo(IStream *destination, ULARGE_INTEGER byteCount,
                                             ULARGE_INTEGER *bytesRead, ULARGE_INTEGER *bytesWritten) override {
                if (!destination)
                    return STG_E_INVALIDPOINTER;
                if (bytesRead)
                    bytesRead->QuadPart = 0;
                if (bytesWritten)
                    bytesWritten->QuadPart = 0;

                std::array<unsigned char, 64 * 1024> buffer{};
                ULONGLONG remaining = byteCount.QuadPart;
                ULONGLONG totalRead = 0;
                ULONGLONG totalWritten = 0;
                while (remaining != 0) {
                    const auto request = static_cast<ULONG>((std::min)(remaining, static_cast<ULONGLONG>(buffer.size())));
                    ULONG currentRead = 0;
                    const auto readResult = Read(buffer.data(), request, &currentRead);
                    if (FAILED(readResult))
                        return readResult;
                    if (currentRead == 0)
                        break;

                    ULONG currentWritten = 0;
                    const auto writeResult = destination->Write(buffer.data(), currentRead, &currentWritten);
                    totalRead += currentRead;
                    totalWritten += currentWritten;
                    remaining -= currentRead;
                    if (bytesRead)
                        bytesRead->QuadPart = totalRead;
                    if (bytesWritten)
                        bytesWritten->QuadPart = totalWritten;
                    if (FAILED(writeResult))
                        return writeResult;
                    if (currentWritten != currentRead)
                        return STG_E_MEDIUMFULL;
                    if (readResult == S_FALSE)
                        break;
                }
                return remaining == 0 ? S_OK : S_FALSE;
            }

            HRESULT STDMETHODCALLTYPE Commit(DWORD) override {
                return S_OK;
            }

            HRESULT STDMETHODCALLTYPE Revert() override {
                return E_NOTIMPL;
            }

            HRESULT STDMETHODCALLTYPE LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) override {
                return STG_E_INVALIDFUNCTION;
            }

            HRESULT STDMETHODCALLTYPE UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) override {
                return STG_E_INVALIDFUNCTION;
            }

            HRESULT STDMETHODCALLTYPE Stat(STATSTG *stat, DWORD) override {
                if (!stat)
                    return STG_E_INVALIDPOINTER;

                QMutexLocker locker(m_mutex.data());
                auto *stream = m_stream.data();
                if (!stream)
                    return STG_E_REVERTED;

                std::memset(stat, 0, sizeof(*stat));
                stat->type = STGTY_STREAM;
                stat->cbSize.QuadPart = static_cast<ULONGLONG>((std::max)(qint64(0), stream->size() - m_offset));
                stat->grfMode = STGM_READ;
                return S_OK;
            }

            HRESULT STDMETHODCALLTYPE Clone(IStream **clonedStream) override {
                if (!clonedStream)
                    return E_POINTER;
                *clonedStream = nullptr;

                auto *clone = new (std::nothrow) QIODeviceIStream(m_stream.data(), m_offset, m_position, m_mutex);
                if (!clone)
                    return E_OUTOFMEMORY;
                *clonedStream = clone;
                return S_OK;
            }

        private:
            QIODeviceIStream(QIODevice *stream, qint64 offset, qint64 position, const QSharedPointer<QMutex> &mutex)
                : m_stream(stream), m_offset(offset), m_position(position), m_mutex(mutex) {
            }

            std::atomic<ULONG> m_referenceCount{1};
            QPointer<QIODevice> m_stream;
            qint64 m_offset = 0;
            qint64 m_position = 0;
            QSharedPointer<QMutex> m_mutex;
        };

        QString makeHResultError(const QString &operation, HRESULT result) {
            return QStringLiteral("%1 (HRESULT 0x%2)")
                .arg(operation)
                .arg(static_cast<quint32>(result), 8, 16, QLatin1Char('0'));
        }

        HRESULT getFloatOutputFormat(IMFSourceReader *reader, DWORD streamIndex, UINT32 &channelCount,
                                     UINT32 &sampleRate, UINT32 &blockAlignment) {
            Microsoft::WRL::ComPtr<IMFMediaType> mediaType;
            auto result = reader->GetCurrentMediaType(streamIndex, mediaType.GetAddressOf());
            if (FAILED(result))
                return result;

            GUID majorType{};
            GUID subtype{};
            result = mediaType->GetGUID(MF_MT_MAJOR_TYPE, &majorType);
            if (FAILED(result))
                return result;
            result = mediaType->GetGUID(MF_MT_SUBTYPE, &subtype);
            if (FAILED(result))
                return result;
            if (majorType != MFMediaType_Audio || subtype != MFAudioFormat_Float)
                return MF_E_INVALIDMEDIATYPE;

            UINT32 bitsPerSample = 0;
            result = mediaType->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &channelCount);
            if (FAILED(result))
                return result;
            result = mediaType->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &sampleRate);
            if (FAILED(result))
                return result;
            result = mediaType->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, &bitsPerSample);
            if (FAILED(result))
                return result;
            result = mediaType->GetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, &blockAlignment);
            if (FAILED(result))
                return result;

            if (channelCount == 0 || sampleRate == 0 || bitsPerSample != 32 ||
                blockAlignment != channelCount * sizeof(float)) {
                return MF_E_INVALIDMEDIATYPE;
            }
            return S_OK;
        }

    }

    void MediaFoundationAACAudioFormatIOPrivate::clearDecodedData() {
        decodedData.clear();
        decodedStartFrame = 0;
        decodedFrameOffset = 0;
        decodedFrameCount = 0;
    }

    void MediaFoundationAACAudioFormatIOPrivate::releaseResources() {
        clearDecodedData();
        sourceReader.Reset();
        if (byteStream)
            byteStream->Close();
        byteStream.Reset();
        inputStream.Reset();

        if (mediaFoundationStarted) {
            MFShutdown();
            mediaFoundationStarted = false;
        }
        if (comInitialized) {
            CoUninitialize();
            comInitialized = false;
        }

        openMode = AbstractAudioFormatIO::NotOpen;
        audioStreamIndex = MF_SOURCE_READER_FIRST_AUDIO_STREAM;
        channelCount = 0;
        sampleRate = 0;
        blockAlignment = 0;
        frameLength = 0;
        framePosition = 0;
        endOfStream = false;
    }

    HRESULT MediaFoundationAACAudioFormatIOPrivate::fillDecodedData(QString &error) {
        clearDecodedData();
        if (endOfStream)
            return S_FALSE;

        for (;;) {
            DWORD actualStreamIndex = 0;
            DWORD flags = 0;
            LONGLONG timestamp = 0;
            Microsoft::WRL::ComPtr<IMFSample> sample;
            auto result = sourceReader->ReadSample(audioStreamIndex, 0, &actualStreamIndex, &flags, &timestamp,
                                                   sample.GetAddressOf());
            if (FAILED(result)) {
                error = QStringLiteral("Failed to read a decoded AAC sample");
                return result;
            }
            if (flags & MF_SOURCE_READERF_ERROR) {
                error = QStringLiteral("The Media Foundation source reader reported a decoding error");
                return E_FAIL;
            }
            if (flags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED) {
                UINT32 newChannelCount = 0;
                UINT32 newSampleRate = 0;
                UINT32 newBlockAlignment = 0;
                result = getFloatOutputFormat(sourceReader.Get(), audioStreamIndex, newChannelCount, newSampleRate,
                                              newBlockAlignment);
                if (FAILED(result)) {
                    error = QStringLiteral("Failed to inspect the changed decoded audio format");
                    return result;
                }
                if (newChannelCount != channelCount || newSampleRate != sampleRate ||
                    newBlockAlignment != blockAlignment) {
                    error = QStringLiteral("The decoded audio format changed while reading");
                    return MF_E_INVALIDMEDIATYPE;
                }
            }
            if (flags & MF_SOURCE_READERF_ENDOFSTREAM)
                endOfStream = true;
            if (!sample) {
                if (endOfStream)
                    return S_FALSE;
                continue;
            }
            if (actualStreamIndex != audioStreamIndex) {
                error = QStringLiteral("The Media Foundation source reader returned an unexpected stream");
                return E_UNEXPECTED;
            }

            Microsoft::WRL::ComPtr<IMFMediaBuffer> buffer;
            result = sample->ConvertToContiguousBuffer(buffer.GetAddressOf());
            if (FAILED(result)) {
                error = QStringLiteral("Failed to make a decoded AAC sample contiguous");
                return result;
            }

            BYTE *data = nullptr;
            DWORD currentLength = 0;
            result = buffer->Lock(&data, nullptr, &currentLength);
            if (FAILED(result)) {
                error = QStringLiteral("Failed to lock a decoded AAC sample");
                return result;
            }
            if (currentLength % blockAlignment != 0 || currentLength > static_cast<DWORD>((std::numeric_limits<int>::max)())) {
                buffer->Unlock();
                error = QStringLiteral("A decoded AAC sample has an invalid byte length");
                return MF_E_INVALIDMEDIATYPE;
            }

            decodedData = QByteArray(reinterpret_cast<const char *>(data), static_cast<int>(currentLength));
            buffer->Unlock();
            decodedStartFrame = static_cast<qint64>(std::llround(
                static_cast<long double>(timestamp) * sampleRate / MediaFoundationTimeScale));
            decodedFrameCount = currentLength / blockAlignment;
            decodedFrameOffset = 0;
            if (decodedFrameCount == 0) {
                clearDecodedData();
                if (endOfStream)
                    return S_FALSE;
                continue;
            }
            return S_OK;
        }
    }

    MediaFoundationAACAudioFormatIO::MediaFoundationAACAudioFormatIO(QIODevice *stream)
        : d_ptr(new MediaFoundationAACAudioFormatIOPrivate) {
        Q_D(MediaFoundationAACAudioFormatIO);
        d->q_ptr = this;
        setStream(stream);
    }

    MediaFoundationAACAudioFormatIO::~MediaFoundationAACAudioFormatIO() {
        MediaFoundationAACAudioFormatIO::close();
    }

    void MediaFoundationAACAudioFormatIO::setStream(QIODevice *stream, qint64 offset) {
        Q_D(MediaFoundationAACAudioFormatIO);
        if (d->openMode) {
            qWarning() << "MediaFoundationAACAudioFormatIO: Cannot set stream while the decoder is open.";
            return;
        }
        d->stream = stream;
        d->streamOffset = offset;
    }

    QIODevice *MediaFoundationAACAudioFormatIO::stream() const {
        Q_D(const MediaFoundationAACAudioFormatIO);
        return d->stream;
    }

    bool MediaFoundationAACAudioFormatIO::open(OpenMode mode) {
        Q_D(MediaFoundationAACAudioFormatIO);
        close();

        bool keepResources = false;
        struct OpenGuard {
            MediaFoundationAACAudioFormatIOPrivate *d;
            bool *keepResources;

            ~OpenGuard() {
                if (!*keepResources)
                    d->releaseResources();
            }
        } guard{d, &keepResources};

        const auto fail = [this](const QString &error) {
            setErrorString(error);
            qWarning().noquote() << "MediaFoundationAACAudioFormatIO:" << error;
            return false;
        };
        const auto failHResult = [&fail](const QString &operation, HRESULT result) {
            return fail(makeHResultError(operation, result));
        };

        if (mode.testFlag(Write))
            return fail(QStringLiteral("Writing is not supported"));
        if (!mode.testFlag(Read))
            return fail(QStringLiteral("Cannot open because read access is not specified"));
        if (!d->stream)
            return fail(QStringLiteral("Cannot open because the stream is null"));

        auto result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        if (SUCCEEDED(result)) {
            d->comInitialized = true;
        } else if (result != RPC_E_CHANGED_MODE) {
            return failHResult(QStringLiteral("Failed to initialize COM"), result);
        }

        result = MFStartup(MF_VERSION, MFSTARTUP_FULL);
        if (FAILED(result))
            return failHResult(QStringLiteral("Failed to initialize Media Foundation"), result);
        d->mediaFoundationStarted = true;

        auto *adapter = new (std::nothrow) QIODeviceIStream(d->stream, d->streamOffset);
        if (!adapter)
            return failHResult(QStringLiteral("Failed to allocate the QIODevice stream adapter"), E_OUTOFMEMORY);
        d->inputStream.Attach(adapter);

        result = MFCreateMFByteStreamOnStream(d->inputStream.Get(), d->byteStream.GetAddressOf());
        if (FAILED(result))
            return failHResult(QStringLiteral("Failed to create the Media Foundation byte stream"), result);

        Microsoft::WRL::ComPtr<IMFSourceResolver> resolver;
        result = MFCreateSourceResolver(resolver.GetAddressOf());
        if (FAILED(result))
            return failHResult(QStringLiteral("Failed to create the Media Foundation source resolver"), result);

        MF_OBJECT_TYPE objectType = MF_OBJECT_INVALID;
        Microsoft::WRL::ComPtr<IUnknown> sourceObject;
        result = resolver->CreateObjectFromByteStream(d->byteStream.Get(), L"stream.mp4", MF_RESOLUTION_MEDIASOURCE,
                                                      nullptr, &objectType, sourceObject.GetAddressOf());
        if (FAILED(result))
            return failHResult(QStringLiteral("Failed to parse the input as an MP4 media source"), result);
        if (objectType != MF_OBJECT_MEDIASOURCE)
            return fail(QStringLiteral("The MP4 source resolver did not return a media source"));

        Microsoft::WRL::ComPtr<IMFMediaSource> mediaSource;
        result = sourceObject.As(&mediaSource);
        if (FAILED(result))
            return failHResult(QStringLiteral("Failed to access the resolved MP4 media source"), result);

        result = MFCreateSourceReaderFromMediaSource(mediaSource.Get(), nullptr, d->sourceReader.GetAddressOf());
        if (FAILED(result))
            return failHResult(QStringLiteral("Failed to create the Media Foundation source reader"), result);

        DWORD selectedStreamIndex = MF_SOURCE_READER_FIRST_AUDIO_STREAM;
        bool foundAAC = false;
        bool foundMP4AAC = false;
        bool reachedLastStream = false;
        for (DWORD streamIndex = 0; !reachedLastStream; ++streamIndex) {
            for (DWORD typeIndex = 0;; ++typeIndex) {
                Microsoft::WRL::ComPtr<IMFMediaType> nativeType;
                result = d->sourceReader->GetNativeMediaType(streamIndex, typeIndex, nativeType.GetAddressOf());
                if (result == MF_E_INVALIDSTREAMNUMBER) {
                    reachedLastStream = true;
                    break;
                }
                if (result == MF_E_NO_MORE_TYPES)
                    break;
                if (FAILED(result))
                    return failHResult(QStringLiteral("Failed to enumerate MP4 media streams"), result);

                GUID majorType{};
                GUID subtype{};
                if (FAILED(nativeType->GetGUID(MF_MT_MAJOR_TYPE, &majorType)) ||
                    FAILED(nativeType->GetGUID(MF_MT_SUBTYPE, &subtype)) || majorType != MFMediaType_Audio ||
                    subtype != MFAudioFormat_AAC) {
                    continue;
                }

                foundAAC = true;
                UINT32 sampleDescriptionSize = 0;
                if (SUCCEEDED(nativeType->GetBlobSize(MF_MT_MPEG4_SAMPLE_DESCRIPTION, &sampleDescriptionSize)) &&
                    sampleDescriptionSize != 0) {
                    selectedStreamIndex = streamIndex;
                    foundMP4AAC = true;
                    break;
                }
            }
            if (foundMP4AAC)
                break;
        }
        if (!foundAAC)
            return fail(QStringLiteral("The MP4 file does not contain an AAC audio stream"));
        if (!foundMP4AAC)
            return fail(QStringLiteral("The AAC audio stream is not contained in an MP4 sample description"));

        result = d->sourceReader->SetStreamSelection(MF_SOURCE_READER_ALL_STREAMS, FALSE);
        if (FAILED(result))
            return failHResult(QStringLiteral("Failed to deselect unused MP4 media streams"), result);
        result = d->sourceReader->SetStreamSelection(selectedStreamIndex, TRUE);
        if (FAILED(result))
            return failHResult(QStringLiteral("Failed to select the first AAC audio stream"), result);

        Microsoft::WRL::ComPtr<IMFMediaType> decodedType;
        result = MFCreateMediaType(decodedType.GetAddressOf());
        if (FAILED(result))
            return failHResult(QStringLiteral("Failed to create the decoded audio media type"), result);
        result = decodedType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
        if (FAILED(result))
            return failHResult(QStringLiteral("Failed to configure the decoded audio major type"), result);
        result = decodedType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_Float);
        if (FAILED(result))
            return failHResult(QStringLiteral("Failed to configure floating-point AAC output"), result);
        result = d->sourceReader->SetCurrentMediaType(selectedStreamIndex, nullptr, decodedType.Get());
        if (FAILED(result))
            return failHResult(QStringLiteral("Failed to enable the Windows AAC decoder"), result);

        result = getFloatOutputFormat(d->sourceReader.Get(), selectedStreamIndex, d->channelCount, d->sampleRate,
                                      d->blockAlignment);
        if (FAILED(result))
            return failHResult(QStringLiteral("Failed to get the decoded floating-point audio format"), result);

        PROPVARIANT duration{};
        PropVariantInit(&duration);
        result = d->sourceReader->GetPresentationAttribute(MF_SOURCE_READER_MEDIASOURCE, MF_PD_DURATION, &duration);
        if (FAILED(result)) {
            PropVariantClear(&duration);
            return failHResult(QStringLiteral("Failed to get the MP4 duration"), result);
        }

        LONGLONG durationValue = -1;
        if (duration.vt == VT_UI8 && duration.uhVal.QuadPart <= static_cast<ULONGLONG>((std::numeric_limits<LONGLONG>::max)()))
            durationValue = static_cast<LONGLONG>(duration.uhVal.QuadPart);
        else if (duration.vt == VT_I8)
            durationValue = duration.hVal.QuadPart;
        PropVariantClear(&duration);
        if (durationValue < 0)
            return fail(QStringLiteral("The MP4 media source returned an invalid duration"));

        d->audioStreamIndex = selectedStreamIndex;
        d->frameLength = MFllMulDiv(durationValue, d->sampleRate, MediaFoundationTimeScale,
                                    MediaFoundationTimeScale / 2);
        d->framePosition = 0;
        d->endOfStream = false;
        d->openMode = Read;
        clearErrorString();
        keepResources = true;
        return true;
    }

    AbstractAudioFormatIO::OpenMode MediaFoundationAACAudioFormatIO::openMode() const {
        Q_D(const MediaFoundationAACAudioFormatIO);
        return d->openMode;
    }

    void MediaFoundationAACAudioFormatIO::close() {
        Q_D(MediaFoundationAACAudioFormatIO);
        d->releaseResources();
        clearErrorString();
    }

    int MediaFoundationAACAudioFormatIO::format() const {
        Q_D(const MediaFoundationAACAudioFormatIO);
        TEST_IS_OPEN(0)
        return AAC;
    }

    void MediaFoundationAACAudioFormatIO::setFormat(int) {
        qWarning() << "MediaFoundationAACAudioFormatIO: Writing is not supported.";
    }

    int MediaFoundationAACAudioFormatIO::channelCount() const {
        Q_D(const MediaFoundationAACAudioFormatIO);
        TEST_IS_OPEN(0)
        return static_cast<int>(d->channelCount);
    }

    void MediaFoundationAACAudioFormatIO::setChannelCount(int) {
        qWarning() << "MediaFoundationAACAudioFormatIO: Writing is not supported.";
    }

    double MediaFoundationAACAudioFormatIO::sampleRate() const {
        Q_D(const MediaFoundationAACAudioFormatIO);
        TEST_IS_OPEN(0.0)
        return d->sampleRate;
    }

    void MediaFoundationAACAudioFormatIO::setSampleRate(double) {
        qWarning() << "MediaFoundationAACAudioFormatIO: Writing is not supported.";
    }

    qint64 MediaFoundationAACAudioFormatIO::length() const {
        Q_D(const MediaFoundationAACAudioFormatIO);
        TEST_IS_OPEN(0)
        return d->frameLength;
    }

    qint64 MediaFoundationAACAudioFormatIO::read(float *ptr, qint64 length) {
        Q_D(MediaFoundationAACAudioFormatIO);
        TEST_IS_OPEN(0)
        if (length <= 0)
            return 0;
        if (!ptr) {
            const auto error = QStringLiteral("Cannot read into a null buffer");
            setErrorString(error);
            qWarning().noquote() << "MediaFoundationAACAudioFormatIO:" << error;
            return 0;
        }
        if (d->framePosition >= d->frameLength)
            return 0;

        const auto requestedFrames = (std::min)(length, d->frameLength - d->framePosition);
        qint64 framesRead = 0;
        while (framesRead < requestedFrames) {
            if (d->decodedFrameOffset >= d->decodedFrameCount) {
                QString operation;
                const auto result = d->fillDecodedData(operation);
                if (result == S_FALSE)
                    break;
                if (FAILED(result)) {
                    const auto error = makeHResultError(operation, result);
                    setErrorString(error);
                    qWarning().noquote() << "MediaFoundationAACAudioFormatIO:" << error;
                    break;
                }
            }

            const auto decodedPosition = d->decodedStartFrame + d->decodedFrameOffset;
            if (decodedPosition < d->framePosition) {
                const auto skippedFrames = (std::min)(d->framePosition - decodedPosition,
                                                      d->decodedFrameCount - d->decodedFrameOffset);
                d->decodedFrameOffset += skippedFrames;
                continue;
            }
            if (decodedPosition > d->framePosition) {
                const auto silentFrames = (std::min)({decodedPosition - d->framePosition,
                                                      requestedFrames - framesRead,
                                                      d->frameLength - d->framePosition});
                std::fill_n(ptr + framesRead * d->channelCount, silentFrames * d->channelCount, 0.0f);
                d->framePosition += silentFrames;
                framesRead += silentFrames;
                continue;
            }

            const auto copiedFrames = (std::min)({d->decodedFrameCount - d->decodedFrameOffset,
                                                  requestedFrames - framesRead,
                                                  d->frameLength - d->framePosition});
            const auto *source = d->decodedData.constData() + d->decodedFrameOffset * d->blockAlignment;
            auto *destination = reinterpret_cast<char *>(ptr + framesRead * d->channelCount);
            std::memcpy(destination, source, static_cast<size_t>(copiedFrames * d->blockAlignment));
            d->decodedFrameOffset += copiedFrames;
            d->framePosition += copiedFrames;
            framesRead += copiedFrames;
        }
        return framesRead;
    }

    qint64 MediaFoundationAACAudioFormatIO::write(const float *, qint64) {
        qWarning() << "MediaFoundationAACAudioFormatIO: Writing is not supported.";
        return 0;
    }

    qint64 MediaFoundationAACAudioFormatIO::seek(qint64 pos) {
        Q_D(MediaFoundationAACAudioFormatIO);
        TEST_IS_OPEN(-1)
        if (pos < 0 || pos > d->frameLength) {
            const auto error = QStringLiteral("The requested audio position is outside the stream");
            setErrorString(error);
            qWarning().noquote() << "MediaFoundationAACAudioFormatIO:" << error;
            return -1;
        }
        if (pos == d->framePosition) {
            clearErrorString();
            return pos;
        }
        if (pos == d->frameLength) {
            d->clearDecodedData();
            d->framePosition = pos;
            d->endOfStream = true;
            clearErrorString();
            return pos;
        }

        PROPVARIANT position{};
        position.vt = VT_I8;
        position.hVal.QuadPart = MFllMulDiv(pos, MediaFoundationTimeScale, d->sampleRate, d->sampleRate / 2);
        const auto result = d->sourceReader->SetCurrentPosition(GUID_NULL, position);
        if (FAILED(result)) {
            const auto error = makeHResultError(QStringLiteral("Failed to seek the AAC audio stream"), result);
            setErrorString(error);
            qWarning().noquote() << "MediaFoundationAACAudioFormatIO:" << error;
            return -1;
        }

        d->clearDecodedData();
        d->framePosition = pos;
        d->endOfStream = false;
        clearErrorString();
        return pos;
    }

    qint64 MediaFoundationAACAudioFormatIO::pos() const {
        Q_D(const MediaFoundationAACAudioFormatIO);
        TEST_IS_OPEN(0)
        return d->framePosition;
    }

}
