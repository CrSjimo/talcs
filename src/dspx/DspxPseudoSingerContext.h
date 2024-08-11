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

#ifndef TALCS_DSPXPSEUDOSINGERCONTEXT_H
#define TALCS_DSPXPSEUDOSINGERCONTEXT_H

#include <QObject>

#include <TalcsDspx/TalcsDspxGlobal.h>

namespace talcs {

    class NoteSynthesizerConfig;

    class DspxTrackContext;
    class DspxSingingClipContext;

    class DspxPseudoSingerContextPrivate;

    class TALCSDSPX_EXPORT DspxPseudoSingerContext : public QObject {
        Q_OBJECT
        Q_DECLARE_PRIVATE(DspxPseudoSingerContext)
        friend class DspxSingingClipContext;
    public:
        explicit DspxPseudoSingerContext(DspxTrackContext *trackContext);
        ~DspxPseudoSingerContext() override;

        DspxTrackContext *trackContext() const;

        DspxSingingClipContext *addSingingClip(int id);
        void removeSingingClip(int id);

        QList<DspxSingingClipContext *> clips() const;

        void setConfig(const NoteSynthesizerConfig &config);
        NoteSynthesizerConfig config() const;

    private:
        QScopedPointer<DspxPseudoSingerContextPrivate> d_ptr;

    };

} // talcs

#endif //TALCS_DSPXPSEUDOSINGERCONTEXT_H
