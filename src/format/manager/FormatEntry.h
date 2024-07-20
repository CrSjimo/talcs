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

#ifndef TALCS_FORMATENTRY_H
#define TALCS_FORMATENTRY_H

#include <QObject>

namespace talcs {

    class AbstractAudioFormatIO;

    class FormatEntry : public QObject {
        Q_OBJECT
    public:
        ~FormatEntry() override;

        virtual QStringList filters() const = 0;
        virtual QStringList extensionHints() const;
        virtual talcs::AbstractAudioFormatIO *getFormatOpen(const QString &filename, QVariant &userData, QWidget *win) = 0;
        virtual talcs::AbstractAudioFormatIO *getFormatLoad(const QString &filename, const QVariant &userData) = 0;

    protected:
        explicit FormatEntry(QObject *parent = nullptr);
    };

} // talcs

#endif //TALCS_FORMATENTRY_H
