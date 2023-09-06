#include "TransportAudioSourceWriter.h"
#include "TransportAudioSourceWriter_p.h"

#include <QMutex>

#include "AudioFormatIO.h"
#include "buffer/InterleavedAudioDataWrapper.h"
#include "source/TransportAudioSource.h"

namespace talcs {

    /**
     * @class TransportAudioSourceWriter
     * @brief The TransportAudioSourceWriter class provides interfaces to write audio.
     *
     * The audio is read from a TransportAudioSource object, and write to an AudioFormatIO object.
     * @see TransportAudioSource, AudioFormatIO
     */


    /**
     * Constructor.
     *
     * TransportAudioSource object and the AudioFormatIO object must be opened before writing is started, and closed
     * manually after writing.
     *
     * Note that this object does not take the ownership of both objects.
     */
    TransportAudioSourceWriter::TransportAudioSourceWriter(TransportAudioSource *src, AudioFormatIO *outFile,
                                                           qint64 startPos, qint64 length)
        : TransportAudioSourceWriter(*new TransportAudioSourceWriterPrivate) {
        Q_D(TransportAudioSourceWriter);
        d->src = src;
        d->pSrc = src->d_func();
        d->outFile = outFile;
        d->startPos = startPos;
        d->length = length;
    }
    TransportAudioSourceWriter::TransportAudioSourceWriter(TransportAudioSourceWriterPrivate &d) : d_ptr(&d) {
        d.q_ptr = this;
    }
    TransportAudioSourceWriter::~TransportAudioSourceWriter() = default;

    /**
     * Starts writing.
     *
     * Note that the function should be called from another thread to avoid from blocking the main thread. It is
     * recommended to move this object to another thread using
     * [QObject::moveToThread](https://doc.qt.io/qt-5/qobject.html#moveToThread)() and connect signals and slots
     * across different threads.
     * @see [QThread](https://doc.qt.io/qt-5/qthread.html)
     */
    void TransportAudioSourceWriter::start() {
        Q_D(TransportAudioSourceWriter);
        {
            QMutexLocker locker(&d->pSrc->mutex);

            qint64 curPos = d->pSrc->position;
            qint64 blockSize = d->src->bufferSize();
            int chCnt = d->outFile->channelCount();

            auto *p = new float[blockSize * chCnt];
            memset(p, 0, sizeof(float) * blockSize * chCnt);
            InterleavedAudioDataWrapper buf(p, chCnt, blockSize);

            d->pSrc->_q_positionAboutToChange(d->startPos);
            d->pSrc->position = d->startPos;
            if (d->pSrc->src)
                d->pSrc->src->setNextReadPosition(d->pSrc->position);

            qint64 framesToRead = d->length;
            while (!d->stopRequested && framesToRead > 0) {
                qint64 readLength = std::min(framesToRead, blockSize);
                if (d->pSrc->src)
                    d->pSrc->src->read({&buf, 0, readLength});
                framesToRead -= readLength;
                d->outFile->write(p, blockSize);
                d->pSrc->_q_positionAboutToChange(d->pSrc->position + readLength);
                d->pSrc->position += readLength;
                emit percentageUpdated(100.0 * (d->length - framesToRead) / d->length);
            }

            delete[] p;

            d->pSrc->_q_positionAboutToChange(curPos);
            d->pSrc->position = curPos;
            if (d->pSrc->src)
                d->pSrc->src->setNextReadPosition(curPos);
        }


        if (d->stopRequested) {
            emit interrupted();
            emit finished(false);
        } else {
            emit completed();
            emit finished(true);
        }
    }

    /**
     * Interrupts the writing process. This function is thread-safe.
     *
     * Note that do not use queued connection to connect this slot to any signal, otherwise this function will not
     * be invoked before completed, since the thread is blocked while writing.
     */
    void TransportAudioSourceWriter::interrupt() {
        Q_D(TransportAudioSourceWriter);
        d->stopRequested = true;
    }

    /**
     * @fn void TransportAudioSourceWriter::percentageUpdated(float percentage)
     * Emitted when each block have been written.
     * @param percentage the percentage of the part finished.
     */

    /**
     * @fn void TransportAudioSourceWriter::completed()
     * Emitted when writing is completed.
     *
     * Note that this signal is emitted after percentageUpdated().
     */

    /**
     * @fn void TransportAudioSourceWriter::interrupted()
     * Emitted when writing is interrupted.
     */

    /**
     * @fn void TransportAudioSourceWriter::finished(bool isCompleted)
     * Emitted when writing finishes, either interrupted or completed.
     *
     * Note that this signal is emitted after completed() or interrupted().
     */
}