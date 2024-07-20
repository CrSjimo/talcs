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

#ifndef TALCS_FORMATMANAGER_H
#define TALCS_FORMATMANAGER_H

#include <TalcsFormat/TalcsFormatGlobal.h>

#include <QObject>

namespace talcs {

    class FormatEntry;

    class AbstractAudioFormatIO;

    class FormatManagerPrivate;

    class TALCSFORMAT_EXPORT FormatManager : public QObject {
        Q_OBJECT
        Q_DECLARE_PRIVATE(FormatManager)
    public:
        explicit FormatManager(QObject *parent = nullptr);
        ~FormatManager() override;

        void addEntry(FormatEntry *entry);
        QList<FormatEntry *> entries() const;
        QStringList extensionHints() const;
        QStringList filters() const;

        FormatEntry *hintFromExtension(const QString &extension) const;

        talcs::AbstractAudioFormatIO *getFormatLoad(const QString &filename, const QVariant &userData) const;

    private:
        QScopedPointer<FormatManagerPrivate> d_ptr;
    };

} // talcs

#endif //TALCS_FORMATMANAGER_H
