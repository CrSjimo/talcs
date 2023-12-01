#ifndef TALCS_BUFFERINGAUDIOSOURCE_P_H
#define TALCS_BUFFERINGAUDIOSOURCE_P_H

#include <TalcsCore/BufferingAudioSource.h>
#include <TalcsCore/private/PositionableAudioSource_p.h>
#include <TalcsCore/AudioBuffer.h>

#include <QMutex>
#include <QRunnable>
#include <QWaitCondition>

namespace talcs {

    class BufferingAudioSourceTask : public QRunnable {
    public:
        explicit BufferingAudioSourceTask(BufferingAudioSourcePrivate *d);
        void run() override;
        void readByFrame(qint64 startPos, qint64 length) const;
        BufferingAudioSourcePrivate *d;
    };

    class BufferingAudioSourcePrivate : public PositionableAudioSourcePrivate {
        Q_DECLARE_PUBLIC(BufferingAudioSource)
    public:
        PositionableAudioSource *src;
        bool takeOwnership;
        int channelCount;
        qint64 readAheadSize;
        QThreadPool *threadPool;

        QMutex mutex;

        AudioBuffer buf;
        QAtomicInteger<qint64> headPosition;
        QAtomicInteger<qint64> tailPosition;

        QWaitCondition bufferingFinished;
        QMutex bufferingTaskMutex;
        QRunnable *currentBufferingTask = nullptr;
        QAtomicInteger<bool> isTerminateRequested = false;
        void commitBufferingTask(bool isCritical);
        void terminateCurrentBufferingTask();
        void accelerateCurrentBufferingTaskAndWait();
    };

}

#endif //TALCS_BUFFERINGAUDIOSOURCE_P_H
