#include "FutureAudioSourceClipSeries.h"
#include "FutureAudioSourceClipSeries_p.h"

#include "FutureAudioSource.h"

namespace talcs {
    FutureAudioSourceClipSeriesPrivate::FutureAudioSourceClipSeriesPrivate(FutureAudioSourceClipSeries *q)
        : AudioSourceClipSeriesImpl(q) {
    }
    bool FutureAudioSourceClipSeriesPrivate::addClip(const FutureAudioSourceClip &clip) {
        // TODO emit signals
        Q_Q(FutureAudioSourceClipSeries);
        if (AudioSourceClipSeriesImpl::addClip(clip)) {
            cachedClipsLength += clip.length();
            QObject::connect(clip.content(), &FutureAudioSource::progressChanged, q, [=](int value) {
                cachedLengthLoaded += (value - clipLengthLoadedDict[clip.position()]);
                clipLengthLoadedDict[clip.position()] = value;
            });
            QObject::connect(clip.content(), &FutureAudioSource::statusChanged, q,
                             [=](FutureAudioSource::Status status) {
                                 if (status == FutureAudioSource::Ready) {
                                     cachedLengthAvailable += clip.length();
                                     clipLengthCachedDict[clip.position()] = true;
                                 }
                             });
            return true;
        }
        return false;
    }
    void FutureAudioSourceClipSeriesPrivate::removeClip(const FutureAudioSourceClip &clip) {
        // TODO emit signals
        Q_Q(FutureAudioSourceClipSeries);
        AudioSourceClipSeriesImpl::removeClip(clip);
        QObject::disconnect(clip.content(), nullptr, q, nullptr);
        cachedClipsLength -= clip.length();
        cachedLengthLoaded -= clipLengthLoadedDict[clip.position()];
        if (clipLengthCachedDict[clip.position()]) {
            cachedLengthAvailable -= clip.length();
        }
        clipLengthLoadedDict.remove(clip.position());
        clipLengthCachedDict.remove(clip.position());
    }
    void FutureAudioSourceClipSeriesPrivate::clearClips() {
        Q_Q(FutureAudioSourceClipSeries);
        for (const auto &clip : q->m_clips) {
            removeClip(clip);
        }
    }

    /**
     * @class FutureAudioSourceClipSeries
     * @brief An AudioClipsSeriesBase object that uses FutureAudioSource
     */

    /**
     * Default constructor.
     */
    FutureAudioSourceClipSeries::FutureAudioSourceClipSeries(QObject *parent)
        : QObject(parent), PositionableAudioSource(*new FutureAudioSourceClipSeriesPrivate(this)) {
    }

    /**
     * Destructor.
     */
    FutureAudioSourceClipSeries::~FutureAudioSourceClipSeries() {
        FutureAudioSourceClipSeries::close();
    }

    qint64 FutureAudioSourceClipSeries::read(const AudioSourceReadData &readData) {
        Q_D(FutureAudioSourceClipSeries);
        QMutexLocker locker(&d->mutex);
        FutureAudioSourceClip readDataInterval(d->position, readData.length);
        if (d->readMode == Notify) {
            if (!canRead(d->position + readData.length, readData.length)) {
                if (!d->isPauseRequiredEmitted) {
                    d->isPauseRequiredEmitted = true;
                    emit pauseRequired();
                }
            } else {
                if (d->isPauseRequiredEmitted) {
                    d->isPauseRequiredEmitted = false;
                    emit resumeRequired();
                }
            }
        }
        for (int ch = 0; ch < readData.buffer->channelCount(); ch++) {
            readData.buffer->clear(ch, readData.startPos, readData.length);
        }
        qAsConst(m_clips).overlap_find_all(
            readDataInterval, [=, &readDataInterval](const decltype(m_clips)::const_iterator &it) {
                auto clip = it->interval();
                auto [clipReadPosition, clipReadInterval] = calculateClipReadData(clip, readDataInterval);
                clip.content()->setNextReadPosition(clipReadPosition);

                if (d->readMode == Block)
                    clip.content()->wait();
                clip.content()->read({
                    readData.buffer,
                    clipReadInterval.position() + readData.startPos,
                    clipReadInterval.length(),
                    readData.silentFlags,
                });
                return true;
            });
        d->position += readData.length;
        return readData.length;
    }

    qint64 FutureAudioSourceClipSeries::length() const {
        return std::numeric_limits<qint64>::max();
    }

    qint64 FutureAudioSourceClipSeries::nextReadPosition() const {
        return PositionableAudioSource::nextReadPosition();
    }

    void FutureAudioSourceClipSeries::setNextReadPosition(qint64 pos) {
        Q_D(FutureAudioSourceClipSeries);
        QMutexLocker locker(&d->mutex);
        PositionableAudioSource::setNextReadPosition(pos);
    }

    bool FutureAudioSourceClipSeries::addClip(const AudioClipBase<FutureAudioSource> &clip) {
        Q_D(FutureAudioSourceClipSeries);
        QMutexLocker locker(&d->mutex);
        if (AudioClipSeriesBase::addClip(clip)) {
            return d->addClip(clip);
        }
        return false;
    }

    bool FutureAudioSourceClipSeries::removeClipAt(qint64 pos) {
        Q_D(FutureAudioSourceClipSeries);
        QMutexLocker locker(&d->mutex);
        auto clip = AudioClipSeriesBase::findClipAt(pos);
        if (AudioClipSeriesBase::removeClipAt(pos)) {
            d->removeClip(clip);
            return true;
        }
        return false;
    }

    void FutureAudioSourceClipSeries::clearClips() {
        Q_D(FutureAudioSourceClipSeries);
        QMutexLocker locker(&d->mutex);
        d->clearClips();
        AudioClipSeriesBase::clearClips();
    }

    bool FutureAudioSourceClipSeries::open(qint64 bufferSize, double sampleRate) {
        Q_D(FutureAudioSourceClipSeries);
        QMutexLocker locker(&d->mutex);
        if (d->open(bufferSize, sampleRate))
            return AudioStreamBase::open(bufferSize, sampleRate);
        return false;
    }

    void FutureAudioSourceClipSeries::close() {
        Q_D(FutureAudioSourceClipSeries);
        QMutexLocker locker(&d->mutex);
        d->close();
        AudioStreamBase::close();
    }

    /**
     * Gets the length of audio that is able to be played within the series (including blank intervals).
     */
    qint64 FutureAudioSourceClipSeries::lengthAvailable() const {
        Q_D(const FutureAudioSourceClipSeries);
        return d->cachedLengthAvailable;
    }

    /**
     * Gets the total length of the audio that is already loaded in each clips.
     */
    qint64 FutureAudioSourceClipSeries::lengthLoaded() const {
        Q_D(const FutureAudioSourceClipSeries);
        return d->cachedLengthLoaded;
    }

    /**
     * Gets the total length of all clips.
     */
    qint64 FutureAudioSourceClipSeries::lengthOfAllClips() const {
        Q_D(const FutureAudioSourceClipSeries);
        return d->cachedClipsLength;
    }

    /**
     * Gets whether a block of audio of a specified length can be read from the series.
     * @param from the start position of reading
     * @param length the length of audio measured in samples
     */
    bool FutureAudioSourceClipSeries::canRead(qint64 from, qint64 length) const {
        Q_D(const FutureAudioSourceClipSeries);
        FutureAudioSourceClip queryInterval(from, length);
        bool flag = true;
        qAsConst(m_clips).overlap_find_all(queryInterval, [=, &flag](const decltype(m_clips)::const_iterator &it) {
            auto clip = it->interval();
            if (clip.content()->status() != FutureAudioSource::Ready) {
                flag = false;
                return false;
            }
            return true;
        });
        return flag;
    }

    /**
     * @enum FutureAudioSourceClipSeries::ReadMode
     * The read modes.
     *
     * @var FutureAudioSourceClipSeries::Notify
     * Default mode. Corresponding signals are emitted when the clip to read is preparing.
     * @see pauseRequired(), resumeRequired()
     *
     * @var FutureAudioSourceClipSeries::Skip
     * The unready clip is skipped.
     *
     * @var FutureAudioSourceClipSeries::Block
     * Waits for the unready clip to finish.
     */

    /**
     * Sets the read mode.
     */
    void FutureAudioSourceClipSeries::setReadMode(FutureAudioSourceClipSeries::ReadMode readMode) {
        Q_D(FutureAudioSourceClipSeries);
        QMutexLocker locker(&d->mutex);
        d->readMode = readMode;
    }

    /**
     * Gets the read mode.
     */
    FutureAudioSourceClipSeries::ReadMode FutureAudioSourceClipSeries::readMode() const {
        Q_D(const FutureAudioSourceClipSeries);
        return d->readMode;
    }

    /**
     * @fn void FutureAudioSourceClipSeries::progressChanged(qint64 lengthAvailable, qint64 lengthLoaded, qint64 lengthOfAllClips, qint64 effectiveLength)
     * Emitted when one of these parameters are changed.
     */

    /**
     * @fn void FutureAudioSourceClipSeries::pauseRequired()
     * Emitted when pause is required in notify mode.
     */

    /**
     * @fn void FutureAudioSourceClipSeries::resumeRequired()
     * Emitted when resume is required in notify mode.
     */

} // talcs