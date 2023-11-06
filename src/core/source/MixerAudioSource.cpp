#include "MixerAudioSource.h"
#include "MixerAudioSource_p.h"

namespace talcs {

    /**
     * @class MixerAudioSource
     * @brief The object that mixes the output of other AudioSource objects.
     * @see PositionableMixerAudioSource
     */

    /**
     * Default constructor.
     */
    MixerAudioSource::MixerAudioSource(QObject *parent) : MixerAudioSource(*new MixerAudioSourcePrivate, parent) {
    }

    MixerAudioSource::MixerAudioSource(MixerAudioSourcePrivate & d, QObject *parent) : QObject(parent), AudioSource(d) {
    }

    /**
     * Destructor.
     *
     * If the object is not close, it will be closed now.
     */
    MixerAudioSource::~MixerAudioSource() {
        Q_D(MixerAudioSource);
        MixerAudioSource::close();
        d->deleteOwnedSources();
    }

    /**
     * @copydoc AudioSource::open()
     *
     * The function also opens all input sources.
     */
    bool MixerAudioSource::open(qint64 bufferSize, double sampleRate) {
        Q_D(MixerAudioSource);
        QMutexLocker locker(&d->mutex);
        if (d->start(bufferSize, sampleRate)) {
            return AudioSource::open(bufferSize, sampleRate);
        } else {
            return false;
        }
    }

    qint64 MixerAudioSource::read(const AudioSourceReadData &readData) {
        Q_D(MixerAudioSource);
        qint64 readLength = readData.length;
        auto channelCount = readData.buffer->channelCount();
        {
            QMutexLocker locker(&d->mutex);
            if (d->tmpBuf.channelCount() < channelCount)
                d->tmpBuf.resize(channelCount);
            for (int i = 0; i < channelCount; i++) {
                readData.buffer->clear(i, readData.startPos, readLength);
            }
            readLength = d->mix(readData, readLength);
        }

        if (d->isMeterEnabled) {
            float magnitude[2] = {0, 0};
            for (int i = 0; i < channelCount; i++) {
                if (i > 1)
                    break;
                magnitude[i] = readData.buffer->magnitude(i, readData.startPos, readLength);
            }
            if (channelCount == 1) {
                magnitude[1] = magnitude[0];
            }
            emit meterUpdated(magnitude[0], magnitude[1]);
        }
        return readLength;
    }

    /**
     * @copydoc AudioSource::close()
     *
     * The function also closes all input sources.
     */
    void MixerAudioSource::close() {
        Q_D(MixerAudioSource);
        QMutexLocker locker(&d->mutex);
        d->stop();
        AudioSource::close();
    }

    bool MixerAudioSource::addSource(AudioSource * src, bool takeOwnership) {
        if (src == this)
            return false;
        Q_D(MixerAudioSource);
        QMutexLocker locker(&d->mutex);
        return d->addSource(src, takeOwnership, isOpen(), bufferSize(), sampleRate());
    }

    bool MixerAudioSource::removeSource(AudioSource * src) {
        Q_D(MixerAudioSource);
        QMutexLocker locker(&d->mutex);
        return d->removeSource(src);
    }

    void MixerAudioSource::removeAllSources() {
        Q_D(MixerAudioSource);
        QMutexLocker locker(&d->mutex);
        d->removeAllSources();
    }

    QList<AudioSource *> MixerAudioSource::sources() const {
        Q_D(const MixerAudioSource);
        return d->sources();
    }

    void MixerAudioSource::setSourceSolo(AudioSource *src, bool isSolo) {
        Q_D(MixerAudioSource);
        QMutexLocker locker(&d->mutex);
        d->setSourceSolo(src, isSolo);
    }

    bool MixerAudioSource::isSourceSolo(AudioSource *src) const {
        Q_D(const MixerAudioSource);
        return d->isSourceSolo(src);
    }

    void MixerAudioSource::setGain(float gain) {
        Q_D(MixerAudioSource);
        QMutexLocker locker(&d->mutex);
        d->gain = gain;
    }

    float MixerAudioSource::gain() const {
        Q_D(const MixerAudioSource);
        return d->gain;
    }

    void MixerAudioSource::setPan(float pan) {
        Q_D(MixerAudioSource);
        QMutexLocker locker(&d->mutex);
        d->pan = pan;
    }

    float MixerAudioSource::pan() const {
        Q_D(const MixerAudioSource);
        return d->pan;
    }

    void MixerAudioSource::setRouteChannels(bool routeChannels) {
        Q_D(MixerAudioSource);
        QMutexLocker locker(&d->mutex);
        d->routeChannels = routeChannels;
    }

    bool MixerAudioSource::routeChannels() const {
        Q_D(const MixerAudioSource);
        return d->routeChannels;
    }

    void MixerAudioSource::setSilentFlags(int silentFlags) {
        Q_D(MixerAudioSource);
        QMutexLocker locker(&d->mutex);
        d->silentFlags = silentFlags;
    }

    int MixerAudioSource::silentFlags() const {
        Q_D(const MixerAudioSource);
        return d->silentFlags;
    }

    void MixerAudioSource::setMeterEnabled(bool enabled) {
        Q_D(MixerAudioSource);
        QMutexLocker locker(&d->mutex);
        d->isMeterEnabled = enabled;
    }

    bool MixerAudioSource::isMeterEnabled() const {
        Q_D(const MixerAudioSource);
        return d->isMeterEnabled;
    }

    /**
     * @fn void MixerAudioSource::meterUpdated(float leftMagnitude, float rightMagnitude)
     * Emitted on each block processed.
     *
     * To use this signal, setMeterEnabled() must be set to true.
     */

}