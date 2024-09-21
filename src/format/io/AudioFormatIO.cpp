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

#include "AudioFormatIO.h"
#include "AudioFormatIO_p.h"

#include <QDebug>
#include <QIODevice>

#include <sndfile.hh>

/**
 * If there is no SndFile handle, this will output a warning message and return the given value.
 * @internal
 */
#define TEST_IS_OPEN(ret)                                                                                              \
    if (d->sf.isNull()) {                                                                                              \
        qWarning() << "AudioFormatIO: Not open.";                                                                      \
        return ret;                                                                                                    \
    }

namespace talcs {

    /**
     * @class AudioFormatIO
     * @brief The AudioFormatIO class provides interfaces to access audio files based on libsndfile.
     *
     * This is one of the @ref doc/object_binding.md "object-binding" classes.
     *
     * @see @link URL https://libsndfile.github.io/libsndfile/ @endlink
     */

    /**
     * Constructor.
     *
     * The open mode of the QIODevice object should be properly set.
     *
     * @param stream the QIODevice to access. This object will not take the ownership of the QIODevice object.
     * @see setStream()
     */
    AudioFormatIO::AudioFormatIO(QIODevice *stream) : AudioFormatIO(*new AudioFormatIOPrivate) {
        setStream(stream);
    }
    AudioFormatIO::AudioFormatIO(AudioFormatIOPrivate &d) : d_ptr(&d) {
        d.q_ptr = this;
    }

    /**
     * Destructor.
     *
     * If the AudioFormatIO object is not close, it will be closed now.
     */
    AudioFormatIO::~AudioFormatIO() {
        AudioFormatIO::close();
    }

    /**
     * Dynamically sets the device.
     *
     * The open mode of the QIODevice object should be properly set.
     *
     * @see AudioFormatIO()
     */
    void AudioFormatIO::setStream(QIODevice *stream) {
        Q_D(AudioFormatIO);
        if (d->openMode) {
            qWarning() << "AudioFormatIO: Cannot set stream when AudioFormatIO is open.";
            return;
        }
        d->stream = stream;
    }

    /**
     * Gets the stream that this object is currently using.
     */
    QIODevice *AudioFormatIO::stream() const {
        Q_D(const AudioFormatIO);
        return d->stream;
    }

    int64_t AudioFormatIOPrivate::sfVioGetFilelen() const {
        return stream->size();
    }
    int64_t AudioFormatIOPrivate::sfVioSeek(int64_t offset, int whence) const {
        if (whence == SF_SEEK_CUR)
            offset += stream->pos();
        else if (whence == SF_SEEK_END)
            offset += stream->size();
        if (offset != stream->pos()) {
            if (!stream->seek(offset))
                return -1;
        }
        return stream->pos();
    }
    int64_t AudioFormatIOPrivate::sfVioRead(void *ptr, int64_t count) const {
        return stream->read(static_cast<char *>(ptr), count);
    }
    int64_t AudioFormatIOPrivate::sfVioWrite(const void *ptr, int64_t count) const {
        return stream->write(static_cast<const char *>(ptr), count);
    }
    int64_t AudioFormatIOPrivate::sfVioTell() const {
        return stream->pos();
    }
    static SF_VIRTUAL_IO sfVio = {
        [](void *d) { return static_cast<AudioFormatIOPrivate *>(d)->sfVioGetFilelen(); },
        [](int64_t offset, int whence, void *d) {
            return static_cast<AudioFormatIOPrivate *>(d)->sfVioSeek(offset, whence);
        },
        [](void *ptr, int64_t count, void *d) {
            return static_cast<AudioFormatIOPrivate *>(d)->sfVioRead(ptr, count);
        },
        [](const void *ptr, int64_t count, void *d) {
            return static_cast<AudioFormatIOPrivate *>(d)->sfVioWrite(ptr, count);
        },
        [](void *d) { return static_cast<AudioFormatIOPrivate *>(d)->sfVioTell(); }};

    /**
     * Initialize libsndfile with specified format, number of channels and sample rate.
     * @return true if successful
     */
    bool AudioFormatIO::open(OpenMode openMode) {
        Q_D(AudioFormatIO);
        close();
        int sfOpenMode = 0;
        if (openMode.testFlag(Read)) {
            sfOpenMode |= SFM_READ;
        }
        if (openMode.testFlag(Write)) {
            sfOpenMode |= SFM_WRITE;
        }
        if (sfOpenMode == 0) {
            qWarning() << "AudioFormatIO: Cannot open because access mode is not specified.";
            setErrorString("AudioFormatIO: Cannot open because access mode is not specified.");
            return false;
        }
        if (!d->stream) {
            qWarning() << "AudioFormatIO: Cannot open because stream is null.";
            setErrorString("AudioFormatIO: Cannot open because stream is null.");
            return false;
        }
        if (!d->stream->openMode()) {
            qWarning() << "AudioFormatIO: Cannot open because stream is not opened.";
            setErrorString("AudioFormatIO: Cannot open because stream is not opened.");
            return false;
        }
        d->stream->seek(0);
        d->sf.reset(new SndfileHandle(sfVio, d, sfOpenMode, d->format, d->channelCount, static_cast<int>(d->sampleRate)));
        if (!d->sf->rawHandle()) {
            setErrorString(d->sf->strError());
            return false;
        }
        d->openMode = openMode;
        clearErrorString();
        return true;
    }

    /**
     * Gets the open mode of the stream.
     */
    AbstractAudioFormatIO::OpenMode AudioFormatIO::openMode() const {
        Q_D(const AudioFormatIO);
        return d->openMode;
    }

    /**
     * Closes the AudioFormatIO object.
     *
     * The stream will also be closed.
     */
    void AudioFormatIO::close() {
        Q_D(AudioFormatIO);
        d->sf.reset();
        d->openMode = NotOpen;
        clearErrorString();
    }

    void AudioFormatIO::setChannelCount(int channelCount) {
        Q_D(AudioFormatIO);
        d->channelCount = channelCount;
    }

    /**
     * Gets the number of channels.
     *
     * Note that this function should be called after the AudioFormatIO object is opened.
     */
    int AudioFormatIO::channelCount() const {
        Q_D(const AudioFormatIO);
        TEST_IS_OPEN(0)
        return d->sf->channels();
    }

    void AudioFormatIO::setSampleRate(double sampleRate) {
        Q_D(AudioFormatIO);
        d->sampleRate = sampleRate;
    }

    /**
     * Gets the sample rate.
     *
     * Note that this function should be called after the AudioFormatIO object is opened.
     */
    double AudioFormatIO::sampleRate() const {
        Q_D(const AudioFormatIO);
        TEST_IS_OPEN(0)
        return d->sf->samplerate();
    }

    /**
     * @enum AudioFormatIO::MajorFormat
     * Major formats.
     * @see @link URL https://libsndfile.github.io/libsndfile/formats.html @endlink
     * @var AudioFormatIO::WAV
     * Microsoft WAV format (little endian default).
     *
     * @var AudioFormatIO::AIFF
     * Apple/SGI AIFF format (big endian).
     *
     * @var AudioFormatIO::AU
     * Sun/NeXT AU format (big endian).
     *
     * @var AudioFormatIO::RAW
     * RAW PCM data.
     *
     * @var AudioFormatIO::PAF
     * Ensoniq PARIS file format.
     *
     * @var AudioFormatIO::SVX
     * Amiga IFF / SVX8 / SV16 format.
     *
     * @var AudioFormatIO::NIST
     * Sphere NIST format.
     *
     * @var AudioFormatIO::VOC
     * VOC files.
     *
     * @var AudioFormatIO::IRCAM
     * Berkeley/IRCAM/CARL
     *
     * @var AudioFormatIO::W64
     * Sonic Foundry's 64 bit RIFF/WAV
     *
     * @var AudioFormatIO::MAT4
     * Matlab (tm) V4.2 / GNU Octave 2.0
     *
     * @var AudioFormatIO::MAT5
     * Matlab (tm) V5.0 / GNU Octave 2.1
     *
     * @var AudioFormatIO::PVF
     * Portable Voice Format
     *
     * @var AudioFormatIO::XI
     * Fasttracker 2 Extended Instrument
     *
     * @var AudioFormatIO::HTK
     * HMM Tool Kit format
     *
     * @var AudioFormatIO::SDS
     * Midi Sample Dump Standard
     *
     * @var AudioFormatIO::AVR
     * Audio Visual Research
     *
     * @var AudioFormatIO::WAVEX
     * MS WAVE with WAVEFORMATEX
     *
     * @var AudioFormatIO::SD2
     * Sound Designer 2
     *
     * @var AudioFormatIO::FLAC
     * FLAC lossless file format
     *
     * @var AudioFormatIO::CAF
     * Core Audio File format
     *
     * @var AudioFormatIO::WVE
     * Psion WVE format
     *
     * @var AudioFormatIO::OGG
     * Xiph OGG container
     *
     * @var AudioFormatIO::MPC2K
     * Akai MPC 2000 sampler
     *
     * @var AudioFormatIO::RF64
     * RF64 WAV file
     *
     * @var AudioFormatIO::MPEG
     * MPEG-1/2 audio stream
     *
     * @var AudioFormatIO::InvalidMajorFormat
     * Invalid major format
     *
     * @var AudioFormatIO::MajorFormatMask
     * Major format mask
     */

    /**
     * @enum AudioFormatIO::Subtype
     * Subtypes.
     * @see @link URL https://libsndfile.github.io/libsndfile/formats.html @endlink
     * @var AudioFormatIO::PCM_S8
     * Signed 8 bit data
     *
     * @var AudioFormatIO::PCM_16
     * Signed 16 bit data
     *
     * @var AudioFormatIO::PCM_24
     * Signed 24 bit data
     *
     * @var AudioFormatIO::PCM_32
     * Signed 32 bit data
     *
     * @var AudioFormatIO::PCM_U8
     * Unsigned 8 bit data (WAV and RAW only)
     *
     * @var AudioFormatIO::FLOAT
     * 32 bit float data
     *
     * @var AudioFormatIO::DOUBLE
     * 64 bit float data
     *
     * @var AudioFormatIO::ULAW
     * U-Law encoded.
     *
     * @var AudioFormatIO::ALAW
     * A-Law encoded.
     *
     * @var AudioFormatIO::IMA_ADPCM
     * IMA ADPCM.
     *
     * @var AudioFormatIO::MS_ADPCM
     * Microsoft ADPCM.
     *
     * @var AudioFormatIO::GSM610
     * GSM 6.10 encoding.
     *
     * @var AudioFormatIO::VOX_ADPCM
     * OKI / Dialogix ADPCM
     *
     * @var AudioFormatIO::NMS_ADPCM_16
     * 16kbs NMS G721-variant encoding.
     *
     * @var AudioFormatIO::NMS_ADPCM_24
     * 24kbs NMS G721-variant encoding.
     *
     * @var AudioFormatIO::NMS_ADPCM_32
     * 32kbs NMS G721-variant encoding.
     *
     * @var AudioFormatIO::G721_32
     * 32kbs G721 ADPCM encoding.
     *
     * @var AudioFormatIO::G723_24
     * 24kbs G723 ADPCM encoding.
     *
     * @var AudioFormatIO::G723_40
     * 40kbs G723 ADPCM encoding.
     *
     * @var AudioFormatIO::DWVW_12
     * 12 bit Delta Width Variable Word encoding.
     *
     * @var AudioFormatIO::DWVW_16
     * 16 bit Delta Width Variable Word encoding.
     *
     * @var AudioFormatIO::DWVW_24
     * 24 bit Delta Width Variable Word encoding.
     *
     * @var AudioFormatIO::DWVW_N
     * N bit Delta Width Variable Word encoding.
     *
     * @var AudioFormatIO::DPCM_8
     * 8 bit differential PCM (XI only)
     *
     * @var AudioFormatIO::DPCM_16
     * 16 bit differential PCM (XI only)
     *
     * @var AudioFormatIO::VORBIS
     * Xiph Vorbis encoding.
     *
     * @var AudioFormatIO::OPUS
     * Xiph/Skype Opus encoding.
     *
     * @var AudioFormatIO::ALAC_16
     * Apple Lossless Audio Codec (16 bit).
     *
     * @var AudioFormatIO::ALAC_20
     * Apple Lossless Audio Codec (20 bit).
     *
     * @var AudioFormatIO::ALAC_24
     * Apple Lossless Audio Codec (24 bit).
     *
     * @var AudioFormatIO::ALAC_32
     * Apple Lossless Audio Codec (32 bit).
     *
     * @var AudioFormatIO::MPEG_LAYER_I
     * MPEG-1 Audio Layer I
     *
     * @var AudioFormatIO::MPEG_LAYER_II
     * MPEG-1 Audio Layer II
     *
     * @var AudioFormatIO::MPEG_LAYER_III
     * MPEG-2 Audio Layer III
     *
     * @var AudioFormatIO::InvalidSubtype
     * Invalid subtype
     *
     * @var AudioFormatIO::SubtypeMask
     * Subtype mask
     */

    /**
     * @enum AudioFormatIO::ByteOrder
     * Byte orders.
     * @var AudioFormatIO::DefaultOrder
     * Default file endian-ness.
     *
     * @var AudioFormatIO::LittleEndian
     * Force little endian-ness.
     *
     * @var AudioFormatIO::BigEndian
     * Force big endian-ness.
     *
     * @var AudioFormatIO::SystemOrder
     * Force CPU endian-ness.
     *
     * @var AudioFormatIO::ByteOrderMask
     * Byte order mask
     */


    void AudioFormatIO::setFormat(int format) {
        Q_D(AudioFormatIO);
        d->format = format;
    }

    /**
     * Gets the format code (the combination of major format, subtype and byte order).
     *
     * Note that this function should be called after the AudioFormatIO object is opened.
     */
    int AudioFormatIO::format() const {
        Q_D(const AudioFormatIO);
        TEST_IS_OPEN(0)
        return d->sf->format();
    }

    /**
     * Gets the major format.
     *
     * Note that this function should be called after the AudioFormatIO object is opened.
     */
    AudioFormatIO::MajorFormat AudioFormatIO::majorFormat() const {
        return static_cast<MajorFormat>(format() & MajorFormatMask);
    }

    /**
     * Gets the subtype.
     *
     * Note that this function should be called after the AudioFormatIO object is opened.
     */
    AudioFormatIO::Subtype AudioFormatIO::subtype() const {
        return static_cast<Subtype>(format() & SubtypeMask);
    }

    /**
     * Gets the byte order.
     *
     * Note that this function should be called after the AudioFormatIO object is opened.
     */
    AudioFormatIO::ByteOrder AudioFormatIO::byteOrder() const {
        return static_cast<ByteOrder>(format() & ByteOrderMask);
    }

    /**
     * Gets the length of the audio measured in samples.
     */
    qint64 AudioFormatIO::length() const {
        Q_D(const AudioFormatIO);
        TEST_IS_OPEN(0)
        return d->sf->frames();
    }

    /**
     * @enum AudioFormatIO::MetaData
     * Types of the meta data.
     * @var AudioFormatIO::Title
     * Title
     *
     * @var AudioFormatIO::Copyright
     * Copyright
     *
     * @var AudioFormatIO::Software
     * Software
     *
     * @var AudioFormatIO::Artist
     * Artist
     *
     * @var AudioFormatIO::Comment
     * Comment
     *
     * @var AudioFormatIO::Date
     * Date
     *
     * @var AudioFormatIO::Album
     * Album
     *
     * @var AudioFormatIO::License
     * License
     *
     * @var AudioFormatIO::TrackNumber
     * Track number
     *
     * @var AudioFormatIO::Genre
     * Genre
     */

    /**
     * Sets the meta data.
     *
     * Note that this function should be called after the AudioFormatIO object is opened.
     */
    void AudioFormatIO::setMetaData(AudioFormatIO::MetaData metaDataType, const QString &str) {
        Q_D(AudioFormatIO);
        TEST_IS_OPEN(void())
        d->sf->setString(metaDataType, str.toUtf8().data());
    }

    /**
     * Gets the meta data.
     *
     * Note that this function should be called after the AudioFormatIO object is opened.
     */
    QString AudioFormatIO::getMetaData(AudioFormatIO::MetaData metaDataType) const {
        Q_D(const AudioFormatIO);
        TEST_IS_OPEN({})
        return QString::fromUtf8(d->sf->getString(metaDataType));
    }

    /**
     * Reads audio data and moves the file pointer.
     * @param ptr pointer to a pre-allocated float array to store audio data in
     * @param length size of audio data to read measured in samples
     * @return the size of audio data actually read.
     */
    qint64 AudioFormatIO::read(float *ptr, qint64 length) {
        Q_D(AudioFormatIO);
        TEST_IS_OPEN(0)
        return d->sf->readf(ptr, length);
    }

    /**
     * Writes audio data and moves the file pointer.
     * @param ptr pointer to a pre-allocated float array that stores audio data
     * @param length size of audio data to write measured in samples
     * @return the size of audio data actually written.
     */
    qint64 AudioFormatIO::write(const float *ptr, qint64 length) {
        Q_D(AudioFormatIO);
        TEST_IS_OPEN(0)
        return d->sf->writef(ptr, length);
    }

    /**
     * Sets the file pointer to a new position.
     * @param pos position measured in samples
     * @return the resulting position if successful, -1 if any error occurs
     */
    qint64 AudioFormatIO::seek(qint64 pos) {
        Q_D(AudioFormatIO);
        TEST_IS_OPEN(0)
        return d->sf->seek(pos, SF_SEEK_SET);
    }

    /**
     * Gets the position of the file pointer measured in samples.
     */
    qint64 AudioFormatIO::pos() const {
        Q_D(const AudioFormatIO);
        TEST_IS_OPEN(0)
        return d->sf->seek(0, SF_SEEK_CUR);
    }

    /**
     * Turn on or off auto clipping when converting float to int.
     */
    void AudioFormatIO::setAutoClip(bool autoClip) {
        Q_D(AudioFormatIO);
        TEST_IS_OPEN(void())
        d->sf->command(SFC_SET_CLIPPING, nullptr, autoClip ? SF_TRUE : SF_FALSE);
    }

    /**
     * Gets whether auto clipping is turned on.
     */
    bool AudioFormatIO::autoClip() const {
        Q_D(const AudioFormatIO);
        TEST_IS_OPEN(false)
        return d->sf->command(SFC_GET_CLIPPING, nullptr, 0) == SF_TRUE;
    }

    /**
     * Sets the compression level.
     *
     * This only takes effect when writing to specified formats.
     * @see @link URL https://libsndfile.github.io/libsndfile/command.html#sfc_set_compression_level @endlink
     */
    void AudioFormatIO::setCompressionLevel(double level) {
        Q_D(AudioFormatIO);
        TEST_IS_OPEN(void())
        Q_ASSERT(0.0 <= level && level <= 1.0);
        d->sf->command(SFC_SET_COMPRESSION_LEVEL, &level, sizeof(double));
        d->compressionLevel = level;
    }

    /**
     * Gets the compression level.
     */
    double AudioFormatIO::compressionLevel() const {
        Q_D(const AudioFormatIO);
        TEST_IS_OPEN(0.0)
        return d->compressionLevel;
    }

    /**
     * @struct AudioFormatIO::SubtypeInfo
     * @brief The specs of a subtype.
     * @var AudioFormatIO::SubtypeInfo::subtype
     * The enum value of subtype
     *
     * @var AudioFormatIO::SubtypeInfo::name
     * The name of subtype
     */

    /**
     * @struct AudioFormatIO::FormatInfo
     * @brief The specs of a major format and all available subtypes.
     * @var AudioFormatIO::FormatInfo::majorFormat
     * The enum value of major format
     *
     * @var AudioFormatIO::FormatInfo::name
     * The name of major format
     *
     * @var AudioFormatIO::FormatInfo::extension
     * The extension in file name
     *
     * @var AudioFormatIO::FormatInfo::subtypes
     * Available subtypes
     *
     * @var AudioFormatIO::FormatInfo::byteOrders
     * Available byte orders
     */

    /**
     * Lists all formats that could be processed.
     * @see @link URL https://libsndfile.github.io/libsndfile/formats.html @endlink
     */
    QList<AudioFormatIO::FormatInfo> AudioFormatIO::availableFormats() {
        QList<FormatInfo> formatInfoList;
        SF_INFO sfinfo = {};
        int simpleFormatCnt, majorTypeCnt, subTypeCnt;
        sf_command(nullptr, SFC_GET_SIMPLE_FORMAT_COUNT, &simpleFormatCnt, sizeof(int));
        sf_command(nullptr, SFC_GET_FORMAT_MAJOR_COUNT, &majorTypeCnt, sizeof(int));
        sf_command(nullptr, SFC_GET_FORMAT_SUBTYPE_COUNT, &subTypeCnt, sizeof(int));
        sfinfo.channels = 1;
        QHash<int, QString> simpleFormatExtensions;
        for (int i = 0; i < simpleFormatCnt; i++) {
            SF_FORMAT_INFO  formatInfo = {};
            formatInfo.format = i;
            sf_command(nullptr, SFC_GET_SIMPLE_FORMAT, &formatInfo, sizeof(formatInfo));
            simpleFormatExtensions.insert(formatInfo.format, formatInfo.extension);
        }
        for (int i = 0; i < majorTypeCnt; i++) {
            SF_FORMAT_INFO info = {};
            FormatInfo formatInfo = {};
            info.format = i;
            sf_command(nullptr, SFC_GET_FORMAT_MAJOR, &info, sizeof(info));
            formatInfo.majorFormat = static_cast<MajorFormat>(info.format);
            formatInfo.name = info.name;
            formatInfo.extension = info.extension;
            for (int j = 0; j < subTypeCnt; j++) {
                info.format = j;
                sf_command(nullptr, SFC_GET_FORMAT_SUBTYPE, &info, sizeof(info));
                sfinfo.format = formatInfo.majorFormat | info.format;
                if (sf_format_check(&sfinfo)) {
                    QStringList subtypeExtensions;
                    auto simpleFormatExtension = simpleFormatExtensions.value(sfinfo.format);
                    if (!simpleFormatExtension.isEmpty() && simpleFormatExtension != formatInfo.extension)
                        subtypeExtensions.append(simpleFormatExtension);
                    auto subtypeExtension = QString(info.extension);
                    if (!subtypeExtension.isEmpty() && subtypeExtension != simpleFormatExtension)
                        subtypeExtensions.append(subtypeExtension);
                    formatInfo.subtypes.append({static_cast<Subtype>(info.format), info.name, subtypeExtensions});
                }
            }
            formatInfo.byteOrders.append(DefaultOrder);
            sfinfo.format = static_cast<int>(formatInfo.majorFormat) | static_cast<int>(formatInfo.subtypes[0].subtype) | LittleEndian;
            if (sf_format_check(&sfinfo))
                formatInfo.byteOrders.append(LittleEndian);
            sfinfo.format = static_cast<int>(formatInfo.majorFormat) | static_cast<int>(formatInfo.subtypes[0].subtype) | BigEndian;
            if (sf_format_check(&sfinfo))
                formatInfo.byteOrders.append(BigEndian);
            if (formatInfo.byteOrders.size() == 3)
                formatInfo.byteOrders.append(SystemOrder);
            formatInfoList.append(formatInfo);
        }
        return formatInfoList;
    }
}
