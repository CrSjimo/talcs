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

#include "WavpackAudioFormatIO.h"
#include "WavpackAudioFormatIO_p.h"

#include <limits>

#include <QFileDevice>
#include <QDebug>

#include <wavpack/wavpack.h>

#define TEST_IS_OPEN(ret)                                                                          \
    if (!d->context) {                                                                             \
        qWarning() << "WavpackAudioFormatIO: Not open.";                                           \
        return ret;                                                                                \
    }

namespace talcs {

    static inline qint32 wpReadBytes(QFileDevice *stream, void *data, qint32 bcount) {
        return stream->read(static_cast<char *>(data), bcount);
    }
    static inline qint32 wpWriteBytes(QFileDevice *stream, void *data, qint32 bcount) {
        return stream->write(static_cast<const char *>(data), bcount);
    }
    static inline qint64 wpGetPos(const QFileDevice *stream) {
        return stream->pos();
    }
    static inline int wpSetPosAbs(QFileDevice *stream, qint64 pos) {
        return stream->seek(pos) ? 0 : -1;
    }
    static inline int wpSetPosRel(QFileDevice *stream, qint64 delta, int mode) {
        if (mode == SEEK_SET) {
            delta += stream->pos();
        } else if (mode == SEEK_END) {
            delta += stream->size();
        }
        return wpSetPosAbs(stream, delta);
    }
    static inline int wpPushBackByte(QFileDevice *stream, int c) {
        return stream->putChar(static_cast<char>(c)) ? c : -1;
    }
    static inline qint64 wpGetLength(const QFileDevice *stream) {
        return stream->size();
    }
    static inline int wpCanSeek(const QFileDevice *stream) {
        return !stream->isSequential();
    }
    static inline int wpTruncateHere(QFileDevice *stream) {
        return stream->resize(stream->pos()) ? 0 : -1;
    }

    static WavpackStreamReader64 wpStreamReader = {
        [](void *id, void *data, int32_t bcount) -> int32_t {
            return wpReadBytes(static_cast<QFileDevice *>(id), data, bcount);
        },
        [](void *id, void *data, int32_t bcount) -> int32_t {
            return wpWriteBytes(static_cast<QFileDevice *>(id), data, bcount);
        },
        [](void *id) -> int64_t {
            return wpGetPos(static_cast<QFileDevice *>(id));
        },
        [](void *id, int64_t pos) -> int {
            return wpSetPosAbs(static_cast<QFileDevice *>(id), pos);
        },
        [](void *id, int64_t pos, int mode) -> int {
            return wpSetPosRel(static_cast<QFileDevice *>(id), pos, mode);
        },
        [](void *id, int c) -> int {
            return wpPushBackByte(static_cast<QFileDevice *>(id), c);
        },
        [](void *id) ->int64_t {
            return wpGetLength(static_cast<QFileDevice *>(id));
        },
        [](void *id) -> int {
            return wpCanSeek(static_cast<QFileDevice *>(id));
        },
        [](void *id) -> int {
            return wpTruncateHere(static_cast<QFileDevice *>(id));
        },
        nullptr,
    };

    WavpackAudioFormatIO::WavpackAudioFormatIO(QFileDevice *stream, QFileDevice *corrStream) : d_ptr(new WavpackAudioFormatIOPrivate) {
        Q_D(WavpackAudioFormatIO);
        d->stream = stream;
        d->corrStream = corrStream;
    }

    WavpackAudioFormatIO::~WavpackAudioFormatIO() {
        WavpackAudioFormatIO::close();
    }

    void WavpackAudioFormatIO::setStream(QFileDevice *stream, QFileDevice *corrStream) {
        Q_D(WavpackAudioFormatIO);
        if (d->openMode) {
            qWarning() << "WavpackAudioFormatIO: Cannot set stream when WavpackAudioFormatIO is open.";
            return;
        }
        d->stream = stream;
        d->corrStream = corrStream;
    }

    QFileDevice *WavpackAudioFormatIO::stream() const {
        Q_D(const WavpackAudioFormatIO);
        return d->stream;
    }

    QFileDevice *WavpackAudioFormatIO::corrStream() const {
        Q_D(const WavpackAudioFormatIO);
        return d->corrStream;
    }

    bool WavpackAudioFormatIO::open(OpenMode mode) {
        Q_D(WavpackAudioFormatIO);
        close();
        if (mode.testFlag(Write)) {
            qWarning() << "WavpackAudioFormatIO: Writing is not supported.";
            return false;
        }
        if (mode == 0) {
            qWarning() << "WavpackAudioFormatIO: Cannot open because access mode is not specified.";
            return false;
        }
        if (!d->stream) {
            qWarning() << "WavpackAudioFormatIO: Cannot open because stream is null.";
            return false;
        }
        if (mode.testFlag(Read)) {
            char err[84];
            int flags = 0;
            if (d->corrStream) {
                flags |= OPEN_WVC;
            }
            flags |= (d->threadCount - 1) << OPEN_THREADS_SHFT;
            auto ctx = WavpackOpenFileInputEx64(&wpStreamReader, d->stream, d->corrStream, err, flags, 0);
            if (!ctx) {
                qWarning() << "WavpackAudioFormatIO: Failed to open" << err;
                setErrorString(err);
                return false;
            }
            d->context = ctx;
        }
        d->openMode = mode;
        return true;
    }

    AbstractAudioFormatIO::OpenMode WavpackAudioFormatIO::openMode() const {
        Q_D(const WavpackAudioFormatIO);
        return d->openMode;
    }

    void WavpackAudioFormatIO::close() {
        Q_D(WavpackAudioFormatIO);
        if (d->context) {
            WavpackCloseFile(d->context);
            d->context = nullptr;
        }
        d->openMode = NotOpen;
        clearErrorString();
    }

    int WavpackAudioFormatIO::format() const {
        Q_D(const WavpackAudioFormatIO);
        TEST_IS_OPEN(0)
        auto mode = WavpackGetMode(d->context);
        if (mode & MODE_FLOAT)
            return Float;
        return WavpackGetBitsPerSample(d->context);
    }

    void WavpackAudioFormatIO::setFormat(int format) {
        qWarning() << "WavpackAudioFormatIO: Writing is not supported.";
    }

    int WavpackAudioFormatIO::channelCount() const {
        Q_D(const WavpackAudioFormatIO);
        TEST_IS_OPEN(0)
        return WavpackGetNumChannels(d->context);
    }

    void WavpackAudioFormatIO::setChannelCount(int channelCount) {
        qWarning() << "WavpackAudioFormatIO: Writing is not supported.";
    }

    double WavpackAudioFormatIO::sampleRate() const {
        Q_D(const WavpackAudioFormatIO);
        TEST_IS_OPEN(0)
        return WavpackGetSampleRate(d->context);
    }

    void WavpackAudioFormatIO::setSampleRate(double sampleRate) {
        qWarning() << "WavpackAudioFormatIO: Writing is not supported.";
    }

    double WavpackAudioFormatIO::bitRate() const {
        Q_D(const WavpackAudioFormatIO);
        TEST_IS_OPEN(0)
        auto mode = WavpackGetMode(d->context);
        if (mode & MODE_LOSSLESS)
            return std::numeric_limits<double>::quiet_NaN();
        return WavpackGetAverageBitrate(d->context, false);
    }

    void WavpackAudioFormatIO::setBitRate(double bitRate) {
        qWarning() << "WavpackAudioFormatIO: Writing is not supported.";
    }

    void WavpackAudioFormatIO::setThreadCount(int threadCount) {
        Q_D(WavpackAudioFormatIO);
        d->threadCount = threadCount;
    }

    int WavpackAudioFormatIO::threadCount() const {
        Q_D(const WavpackAudioFormatIO);
        return d->threadCount;
    }

    qint64 WavpackAudioFormatIO::length() const {
        Q_D(const WavpackAudioFormatIO);
        TEST_IS_OPEN(0)
        return WavpackGetNumSamples64(d->context);
    }

    static inline void convertSamplesInPlaceToFloat(float *p, qsizetype size, int bitDepth) {
        const double NORM = ((1 << (bitDepth - 1)) - 1) + 0.49999;
        for (float *pp = p; pp < p + size; pp++) {
            *pp = static_cast<float>(*reinterpret_cast<qint32 *>(pp) / NORM);
        }
    }

    qint64 WavpackAudioFormatIO::read(float *ptr, qint64 length) {
        Q_D(WavpackAudioFormatIO);
        TEST_IS_OPEN(0)
        auto ret = WavpackUnpackSamples(d->context, reinterpret_cast<qint32 *>(ptr), static_cast<quint32>(length));
        auto fmt = format();
        if (fmt == Float) {
            return ret;
        } else {
            convertSamplesInPlaceToFloat(ptr, length * channelCount(), fmt);
            return ret;
        }
    }

    qint64 WavpackAudioFormatIO::write(const float *ptr, qint64 length) {
        qWarning() << "WavpackAudioFormatIO: Writing is not supported.";
        return 0;
    }

    qint64 WavpackAudioFormatIO::seek(qint64 pos) {
        Q_D(WavpackAudioFormatIO);
        return WavpackSeekSample64(d->context, pos);
    }

    qint64 WavpackAudioFormatIO::pos() const {
        Q_D(const WavpackAudioFormatIO);
        return WavpackGetSampleIndex64(d->context);
    }

}
