#include "AudioFormatInputSource.h"
#include "AudioFormatInputSource_p.h"

#include <QDebug>

#include <TalcsCore/InterleavedAudioDataWrapper.h>
#include <TalcsFormat/AudioFormatIO.h>

namespace talcs {

    /**
     * @class AudioFormatInputSource
     * @brief Takes audio from an AudioFormatIO object, resamples it to a specified sample rate, and produces it out.
     *
     * libsamplerate is used to resample the audio.
     * @see @link URL https://libsndfile.github.io/libsamplerate/ @endlink
     */

    /**
     * Constructor.
     *
     * Note that unlike other objects, one AudioFormatIO object can be set to multiple AudioFormatInputSource object,
     * and the ownership is not taken.
     */
    AudioFormatInputSource::AudioFormatInputSource(AudioFormatIO *audioFormatIo)
        : AudioFormatInputSource(*new AudioFormatInputSourcePrivate) {
        setAudioFormatIo(audioFormatIo);
    }
    AudioFormatInputSource::~AudioFormatInputSource() = default;
    AudioFormatInputSource::AudioFormatInputSource(AudioFormatInputSourcePrivate &d) : PositionableAudioSource(d) {
    }

    void AudioFormatInputSourcePrivate::resizeInDataBuffers(qint64 bufferSize) {
        inData.resize(io->channelCount() * (qint64) (bufferSize / ratio));
    }
    void AudioFormatInputSourcePrivate::resizeOutDataBuffers(qint64 bufferSize) {
        outData.resize(io->channelCount() * bufferSize);
    }
    long AudioFormatInputSourcePrivate::fetchInData(float **data) {
        Q_Q(AudioFormatInputSource);
        *data = inData.data();
        io->seek(inPosition);
        auto inLength = io->read(inData.data(), q->bufferSize() / ratio);
        inPosition += inLength;
        return inLength;
    }

    qint64 AudioFormatInputSource::read(const AudioSourceReadData &readData) {
        Q_D(AudioFormatInputSource);
        QMutexLocker locker(&d->mutex);
        Q_ASSERT(d->io && isOpen());
        auto readLength = std::min(readData.length, length() - d->position);
        if (readLength > bufferSize())
            d->resizeOutDataBuffers(readLength);
        src_callback_read(d->srcState, d->ratio, readLength, d->outData.data());
        InterleavedAudioDataWrapper outBuf(d->outData.data(), d->io->channelCount(), readLength);
        int channelCount = std::min(d->io->channelCount(), readData.buffer->channelCount());
        for (int ch = 0; ch < channelCount; ch++) {
            readData.buffer->setSampleRange(ch, readData.startPos, readLength, outBuf, ch, 0);
        }
        if (d->doStereoize && d->io->channelCount() == 1 && readData.buffer->channelCount() > 1) {
            readData.buffer->setSampleRange(1, readData.startPos, readLength, outBuf, 0, 0);
        }
        d->position += readLength;
        return readLength;
    }
    qint64 AudioFormatInputSource::length() const {
        Q_D(const AudioFormatInputSource);
        if (!d->io || !isOpen())
            return 0;
        return d->io->length() * d->ratio;
    }
    void AudioFormatInputSource::setNextReadPosition(qint64 pos) {
        Q_D(AudioFormatInputSource);
        QMutexLocker locker(&d->mutex);
        if (pos != d->position) {
            src_reset(d->srcState);
            if (d->io && d->io->openMode())
                d->io->seek(pos / d->ratio);
            d->inPosition = pos / d->ratio;
        }
        PositionableAudioSource::setNextReadPosition(pos);
    }
    bool AudioFormatInputSource::open(qint64 bufferSize, double sampleRate) {
        Q_D(AudioFormatInputSource);
        QMutexLocker locker(&d->mutex);
        if (!d->io)
            return false;
        if (d->io->open(QIODevice::ReadOnly)) {
            d->ratio = sampleRate / d->io->sampleRate();
            int srcError = 0;
            d->srcState = src_callback_new(
                [](void *cbData, float **data) {
                    return reinterpret_cast<AudioFormatInputSourcePrivate *>(cbData)->fetchInData(data);
                },
                d->resampleMode, d->io->channelCount(), &srcError, d);
            if (srcError) {
                qWarning() << src_strerror(srcError);
                return false;
            }
            srcError = src_set_ratio(d->srcState, d->ratio);
            if (srcError) {
                qWarning() << src_strerror(srcError);
                return false;
            }
            d->resizeInDataBuffers(bufferSize);
            d->resizeOutDataBuffers(bufferSize);
            d->io->seek(d->position / d->ratio);
            return AudioStreamBase::open(bufferSize, sampleRate);
        } else
            return false;
    }
    void AudioFormatInputSource::close() {
        Q_D(AudioFormatInputSource);
        QMutexLocker locker(&d->mutex);
        if (!d->io)
            return;
        d->io->close();
        d->ratio = 0;
        if (d->srcState)
            src_delete(d->srcState);
        d->srcState = nullptr;
    }

    /**
     * Resets the internal AudioFormatIO object to another one.
     *
     * Note that this function should not be called when the source is open.
     */
    void AudioFormatInputSource::setAudioFormatIo(AudioFormatIO *audioFormatIo) {
        Q_ASSERT(!isOpen());
        if (isOpen()) {
            qWarning() << "Cannot set audio format io when source is opened.";
            return;
        }
        Q_D(AudioFormatInputSource);
        QMutexLocker locker(&d->mutex);
        d->io = audioFormatIo;
        if (d->io && d->io->openMode())
            d->io->seek(d->inPosition);
        if (d->srcState)
            src_reset(d->srcState);
    }

    /**
     * Gets the AudioFormatIO that is currently being used.
     */
    AudioFormatIO *AudioFormatInputSource::audioFormatIo() const {
        Q_D(const AudioFormatInputSource);
        return d->io;
    }

    /**
     * Clears the internal buffer of the resampler.
     *
     * @note The resampler will be flushed when the read position is changed.
     */
    void AudioFormatInputSource::flush() {
        Q_D(AudioFormatInputSource);
        QMutexLocker locker(&d->mutex);
        if (d->srcState)
            src_reset(d->srcState);
    }

    /**
     * Sets whether this source produces stereo audio when the AudioFormatIO outputs mono.
     */
    void AudioFormatInputSource::setStereoize(bool stereoize) {
        Q_D(AudioFormatInputSource);
        QMutexLocker locker(&d->mutex);
        d->doStereoize = stereoize;
    }

    /**
     * Gets whether this source produces stereo audio when the AudioFormatIO outputs mono.
     */
    bool AudioFormatInputSource::stereoize() const {
        Q_D(const AudioFormatInputSource);
        return d->doStereoize;
    }

    /**
     * @enum AudioFormatInputSource::ResampleMode
     * Resample modes.
     * @note It is recommended to use faster modes for realtime processing and higher-quality modes for non-realtime
     * processing.
     * @see @link URL https://libsndfile.github.io/libsamplerate/api_misc.html#converters @endlink
     * @var AudioFormatInputSource::SincBestQuality
     * This is a bandlimited interpolator derived from the mathematical sinc function and this is the highest quality
     * sinc based converter, providing a worst case Signal-to-Noise Ratio (SNR) of 97 decibels (dB) at a bandwidth of
     * 97%. All three SRC_SINC_* converters are based on the techniques of Julius O. Smith although this code was
     * developed independently.
     *
     * @var AudioFormatInputSource::SincMediumQuality
     * This is another bandlimited interpolator much like the previous one. It has an SNR of 97dB and a bandwidth of
     * 90%. The speed of the conversion is much faster than the previous one.
     *
     * @var AudioFormatInputSource::SincFastest
     * This is the fastest bandlimited interpolator and has an SNR of 97dB and a bandwidth of 80%.
     *
     * @var AudioFormatInputSource::ZeroOrderHold
     * A Zero Order Hold converter (interpolated value is equal to the last value). The quality is poor but the
     * conversion speed is blindlingly fast. Be aware that this interpolator is not bandlimited, and the user is
     * responsible for adding anti-aliasing filtering.
     *
     * @var AudioFormatInputSource::Linear
     * A linear converter. Again the quality is poor, but the conversion speed is blindingly fast. This interpolator is
     * also not bandlimited, and the user is responsible for adding anti-aliasing filtering.
     */

    /**
     * Sets the mode of resampler.
     *
     * Note that this function should be called when the source is not open. If it is called when the source is open, it
     * will take effect the next time the source is opened.
     */
    void AudioFormatInputSource::setResamplerMode(AudioFormatInputSource::ResampleMode mode) {
        Q_D(AudioFormatInputSource);
        d->resampleMode = mode;
    }

    /**
     * Gets the mode of resampler.
     */
    AudioFormatInputSource::ResampleMode AudioFormatInputSource::resampleMode() const {
        Q_D(const AudioFormatInputSource);
        return d->resampleMode;
    }
}