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

#ifndef TALCS_DSPXSINGINGCLIPCONTEXT_H
#define TALCS_DSPXSINGINGCLIPCONTEXT_H

#include <QObject>

#include <TalcsDspx/TalcsDspxGlobal.h>

namespace talcs {

    class PositionableMixerAudioSource;
    class DspxPseudoSingerContext;
    class DspxNoteContext;
    class DspxNoteContextPrivate;

    class DspxSingingClipContextPrivate;

    class TALCSDSPX_EXPORT DspxSingingClipContext : public QObject {
        Q_OBJECT
        Q_DECLARE_PRIVATE(DspxSingingClipContext)
        friend class DspxPseudoSingerContext;
        friend class DspxNoteContext;
        friend class DspxNoteContextPrivate;
    public:
        ~DspxSingingClipContext() override;

        DspxPseudoSingerContext *pseudoSingerContext() const;

        PositionableMixerAudioSource *controlMixer() const;

        void setStart(int tick);
        int start() const;

        void setClipStart(int tick);
        int clipStart() const;

        void setClipLen(int tick);
        int clipLen() const;

        void updatePosition();

        void setData(const QVariant &data);
        QVariant data() const;

        DspxNoteContext *addNote(int id);
        void removeNote(int id);

        QList<DspxNoteContext *> notes() const;

    private:
        explicit DspxSingingClipContext(DspxPseudoSingerContext *pseudoSingerContext);
        QScopedPointer<DspxSingingClipContextPrivate> d_ptr;

    };

} // talcs

#endif //TALCS_DSPXSINGINGCLIPCONTEXT_H
