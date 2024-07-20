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

#ifndef TALCS_DSPXTRACKCONTEXT_H
#define TALCS_DSPXTRACKCONTEXT_H

#include <QObject>

#include <TalcsDspx/TalcsDspxGlobal.h>

namespace talcs {

    class PositionableMixerAudioSource;
    class AudioSourceClipSeries;

    class DspxProjectContext;
    class DspxAudioClipContext;

    class DspxTrackContextPrivate;

    class TALCSDSPX_EXPORT DspxTrackContext : public QObject {
        Q_OBJECT
        Q_DECLARE_PRIVATE(DspxTrackContext)
    public:
        explicit DspxTrackContext(DspxProjectContext *projectContext);
        ~DspxTrackContext() override;

        PositionableMixerAudioSource *controlMixer() const;
        PositionableMixerAudioSource *trackMixer() const;
        AudioSourceClipSeries *clipSeries() const;

        DspxProjectContext *projectContext() const;

        void setData(const QVariant &data);
        QVariant data() const;

        DspxAudioClipContext *addAudioClip(int id);
        void removeAudioClip(int id);

        QList<DspxAudioClipContext *> clips() const;

    private:
        QScopedPointer<DspxTrackContextPrivate> d_ptr;
    };

} // talcs

#endif //TALCS_DSPXTRACKCONTEXT_H
