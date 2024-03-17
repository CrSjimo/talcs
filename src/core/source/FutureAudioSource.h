/******************************************************************************
 * Copyright (c) 2023-2024 CrSjimo                                            *
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

#ifndef TALCS_FUTUREAUDIOSOURCE_H
#define TALCS_FUTUREAUDIOSOURCE_H

#include <QFuture>

#include <TalcsCore/PositionableAudioSource.h>
#include <TalcsCore/TalcsCoreGlobal.h>

namespace talcs {

    class FutureAudioSourcePrivate;

    class TALCSCORE_EXPORT FutureAudioSource : public QObject, public PositionableAudioSource {
        Q_OBJECT
        Q_DECLARE_PRIVATE_D(PositionableAudioSource::d_ptr, FutureAudioSource)
    public:
        struct Callbacks {
            Callbacks()
                : preloadingOpen([](qint64, double) { return true; }), preloadingClose([]() {}) {
            }
            std::function<bool(qint64, double)> preloadingOpen;
            std::function<void()> preloadingClose;
        };

        explicit FutureAudioSource(const QFuture<PositionableAudioSource *> &future,
                                   const Callbacks &callbacks = {}, QObject *parent = nullptr);
        ~FutureAudioSource() override;

        QFuture<PositionableAudioSource *> future() const;
        void setFuture(const QFuture<PositionableAudioSource *> &future);

        qint64 read(const AudioSourceReadData &readData) override;
        qint64 length() const override;
        qint64 nextReadPosition() const override;
        void setNextReadPosition(qint64 pos) override;
        bool open(qint64 bufferSize, double sampleRate) override;
        void close() override;

        int progress() const;

        void pause();
        void resume();
        void cancel();

        enum Status {
            Running,
            Paused,
            Cancelled,
            Ready,
        };
        Status status() const;

        void wait();
        PositionableAudioSource *source() const;

    signals:
        void statusChanged(Status status);
        void progressChanged(int progress);
    };

}

#endif // TALCS_FUTUREAUDIOSOURCE_H
