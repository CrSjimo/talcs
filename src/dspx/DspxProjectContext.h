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

#ifndef TALCS_DSPXPROJECTCONTEXT_H
#define TALCS_DSPXPROJECTCONTEXT_H

#include <functional>

#include <QObject>

#include <TalcsDspx/TalcsDspxGlobal.h>

namespace talcs {

    class MixerAudioSource;
    class TransportAudioSource;
    class PositionableMixerAudioSource;
    class PositionableAudioSource;
    class BufferingAudioSource;

    class FormatManager;

    class DspxTrackContext;

    class DspxProjectContextPrivate;

    class TALCSDSPX_EXPORT DspxProjectContext : public QObject {
        Q_OBJECT
        Q_DECLARE_PRIVATE(DspxProjectContext)
    public:
        explicit DspxProjectContext(QObject *parent = nullptr);
        ~DspxProjectContext() override;

        MixerAudioSource *preMixer() const;
        TransportAudioSource *transport() const;
        PositionableMixerAudioSource *postMixer() const;
        PositionableMixerAudioSource *masterControlMixer() const;
        PositionableMixerAudioSource *masterTrackMixer() const;

        void setFormatManager(FormatManager *formatManager);
        FormatManager *formatManager() const;

        void setTimeConverter(const std::function<qint64(int)> &converter);
        std::function<qint64(int)> timeConverter() const;

        void setBufferingReadAheadSize(qint64 size);
        qint64 bufferingReadAheadSize() const;
        BufferingAudioSource *makeBufferable(PositionableAudioSource *source, int channelCount);

        DspxTrackContext *addTrack(int index);
        void removeTrack(int index);
        void moveTrack(int index, int count, int dest);

        QList<DspxTrackContext *> tracks() const;

    private:
        QScopedPointer<DspxProjectContextPrivate> d_ptr;
    };

} // talcs

#endif //TALCS_DSPXPROJECTCONTEXT_H
