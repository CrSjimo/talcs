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

#ifndef TALCS_DSPXNOTECONTEXT_H
#define TALCS_DSPXNOTECONTEXT_H

#include <QObject>

#include <TalcsDspx/TalcsDspxGlobal.h>

namespace talcs {

    class DspxSingingClipContext;

    class DspxNoteContextPrivate;

    class TALCSDSPX_EXPORT DspxNoteContext : public QObject {
        Q_OBJECT
        Q_DECLARE_PRIVATE(DspxNoteContext)
        friend class DspxSingingClipContext;
    public:
        ~DspxNoteContext() override;

        DspxSingingClipContext *singingClipContext() const;

        void setPos(int pos);
        int pos() const;

        void setLength(int length);
        int length() const;

        void updatePosition();

        void setKeyCent(int cent);
        int keyCent() const;

        void addPitchAnchor(int pos, const QVariant &anchorData);
        void removePitchAnchor(int pos);

        void addEnergyAnchor(int pos, const QVariant &anchorData);
        void removeEnergyAnchor(int pos);

    private:
        explicit DspxNoteContext(DspxSingingClipContext *singingClipContext);
        QScopedPointer<DspxNoteContextPrivate> d_ptr;
    };

} // talcs

#endif //TALCS_DSPXNOTECONTEXT_H
