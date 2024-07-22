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

#ifndef TALCS_DSPXPROJECTCONTEXT_P_H
#define TALCS_DSPXPROJECTCONTEXT_P_H

#include <memory>

#include <TalcsCore/BufferingAudioSource.h>
#include <TalcsCore/MixerAudioSource.h>
#include <TalcsCore/PositionableMixerAudioSource.h>
#include <TalcsCore/TransportAudioSource.h>

#include <TalcsDspx/DspxProjectContext.h>

namespace talcs {

    class DspxProjectContextBufferingAudioSourceObject : public QObject, public BufferingAudioSource {
        Q_OBJECT
    public:
        explicit DspxProjectContextBufferingAudioSourceObject(PositionableAudioSource *src, int channelCount, qint64 readAheadSize, QObject *parent = nullptr);
        ~DspxProjectContextBufferingAudioSourceObject() override;
    };

    class DspxProjectContextPrivate : public QObject {
        Q_OBJECT
        Q_DECLARE_PUBLIC(DspxProjectContext);
    public:
        DspxProjectContext *q_ptr;

        std::unique_ptr<PositionableMixerAudioSource> masterTrackMixer;
        std::unique_ptr<PositionableMixerAudioSource> masterControlMixer;
        std::unique_ptr<PositionableMixerAudioSource> postMixer;
        std::unique_ptr<TransportAudioSource> transport;
        std::unique_ptr<MixerAudioSource> preMixer;

        FormatManager *formatManager = nullptr;
        std::function<qint64(int)> timeConverter = [](int) { return 0; };
        qint64 bufferingReadAheadSize = 0;

        QList<DspxTrackContext *> tracks;

    signals:
        void readAheadSizeChanged(qint64 readAheadSize);
    };
}

#endif //TALCS_DSPXPROJECTCONTEXT_P_H
