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

#ifndef TALCS_STANDARDFORMATENTRY_H
#define TALCS_STANDARDFORMATENTRY_H

#include <TalcsFormat/FormatEntry.h>
#include <TalcsWidgets/TalcsWidgetsGlobal.h>

namespace talcs {

    class StandardFormatEntryPrivate;

    class TALCSWIDGETS_EXPORT StandardFormatEntry : public FormatEntry {
        Q_OBJECT
        Q_DECLARE_PRIVATE(StandardFormatEntry)
    public:
        explicit StandardFormatEntry(QObject *parent = nullptr);
        ~StandardFormatEntry() override;

        QStringList filters() const override;
        QStringList extensionHints() const override;
        AbstractAudioFormatIO *getFormatOpen(const QString &filename, QVariant &userData, QWidget *win) override;
        AbstractAudioFormatIO *getFormatLoad(const QString &filename, const QVariant &userData) override;

    private:
        QScopedPointer<StandardFormatEntryPrivate> d_ptr;
    };

}

#endif //TALCS_STANDARDFORMATENTRY_H
