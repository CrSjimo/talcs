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

#ifndef TALCS_AUDIOSOURCEPROCESSORBASE_H
#define TALCS_AUDIOSOURCEPROCESSORBASE_H

#include <QObject>

#include <TalcsCore/TalcsCoreGlobal.h>

namespace talcs {

    class AudioSourceProcessorBasePrivate;
    class AudioSource;
    class IAudioSampleContainer;

    class TALCSCORE_EXPORT AudioSourceProcessorBase: public QObject {
        Q_OBJECT
        Q_DECLARE_PRIVATE(AudioSourceProcessorBase)
    public:
        AudioSourceProcessorBase(AudioSource *src, qint64 length, QObject *parent = nullptr);
        ~AudioSourceProcessorBase() override;

        enum Status {
            Ready,
            Processing,
            Failed,
            Interrupted,
            Completed,
        };
        Status status() const;

        AudioSource *source() const;
        qint64 length() const;
        void interrupt();

    public slots:
        void start();

    signals:
        void blockProcessed(qint64 processedSampleCount);
        void finished();

    protected:
        AudioSourceProcessorBase(AudioSourceProcessorBasePrivate &d, AudioSource *src, qint64 length, QObject *parent);
        virtual IAudioSampleContainer *prepareBuffer() = 0;
        virtual bool processBlock(qint64 processedSampleCount, qint64 samplesToProcess) = 0;
        virtual void processWillFinish() = 0;

        QScopedPointer<AudioSourceProcessorBasePrivate> d_ptr;
    };

}

#endif // TALCS_AUDIOSOURCEPROCESSORBASE_H
