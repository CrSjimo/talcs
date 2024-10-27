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

#ifndef TALCS_DSPXAUDIOCLIPCONTEXT_H
#define TALCS_DSPXAUDIOCLIPCONTEXT_H

#include <QObject>
#include <QVariant>

#include <TalcsDspx/TalcsDspxGlobal.h>

namespace talcs {

    class PositionableMixerAudioSource;
    class BufferingAudioSource;
    class AbstractAudioFormatIO;

    class DspxTrackContext;

    class DspxAudioClipContextPrivate;

    class TALCSDSPX_EXPORT DspxAudioClipContext : public QObject {
        Q_OBJECT
        Q_DECLARE_PRIVATE(DspxAudioClipContext)
        friend class DspxTrackContext;
    public:
        ~DspxAudioClipContext() override;

        PositionableMixerAudioSource *controlMixer() const;
        PositionableMixerAudioSource *clipMixer() const;
        BufferingAudioSource *contentSource() const;

        DspxTrackContext *trackContext() const;

        void setStart(int tick);
        int start() const;

        void setClipStart(int tick);
        int clipStart() const;

        void setClipLen(int tick);
        int clipLen() const;

        bool setPathLoad(const QString &path, const QVariant &data = {}, const QString &entryClassName = {});
        bool setPathOpen(const QString &path, AbstractAudioFormatIO *io);
        QString path() const;

        void updatePosition();

        void setData(const QVariant &data);
        QVariant data() const;

    private:
        explicit DspxAudioClipContext(DspxTrackContext *trackContext);
        QScopedPointer<DspxAudioClipContextPrivate> d_ptr;

    };

} // talcs

#endif //TALCS_DSPXAUDIOCLIPCONTEXT_H
