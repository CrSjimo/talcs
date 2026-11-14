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

#ifndef TALCS_BUFFERINGAUDIOSOURCE_P_H
#define TALCS_BUFFERINGAUDIOSOURCE_P_H

#include <TalcsCore/BufferingAudioSource.h>

#include <QAtomicInteger>
#include <QMutex>
#include <QRunnable>
#include <QWaitCondition>

#include <TalcsCore/AudioBuffer.h>
#include <TalcsCore/private/PositionableAudioSource_p.h>

namespace talcs {

    class BufferingAudioSourceTask : public QRunnable {
    public:
        explicit BufferingAudioSourceTask(BufferingAudioSourcePrivate *d);
        void run() override;
        BufferingAudioSourcePrivate *d;
    };

    class BufferingAudioSourcePrivate : public PositionableAudioSourcePrivate {
        Q_DECLARE_PUBLIC(BufferingAudioSource)
    public:
        PositionableAudioSource *src;
        bool takeOwnership;
        int channelCount;
        qint64 readAheadSize;
        bool autoBuffering;
        QThreadPool *threadPool;

        QRecursiveMutex mutex;

        QMutex bufLock;
        AudioBuffer buf;
        qint64 headPosition = 0;
        QAtomicInteger<qint64> tailPosition = 0;

        QWaitCondition bufferingFinished;
        QMutex bufferingTaskMutex;
        BufferingAudioSourceTask bufferingTask{this};
        bool bufferingTaskPending = false;
        quint64 completedBufferingTasks = 0;
        QAtomicInteger<bool> isTerminateRequested = false;
        void fillBuffer(qint64 targetLength);
        void commitBufferingTask();
        void finishBufferingTask();
        void terminateCurrentBufferingTask();
    };

}

#endif //TALCS_BUFFERINGAUDIOSOURCE_P_H
