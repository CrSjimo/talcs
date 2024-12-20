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

#ifndef TALCS_WAVPACKFORMATENTRY_H
#define TALCS_WAVPACKFORMATENTRY_H

#include <TalcsFormat/FormatEntry.h>
#include <TalcsWidgets/TalcsWidgetsGlobal.h>

namespace talcs {

class TALCSWIDGETS_EXPORT WavpackFormatEntry : public FormatEntry {
    Q_OBJECT
public:
    explicit WavpackFormatEntry(QObject *parent = nullptr);
    ~WavpackFormatEntry() override;

    QStringList filters() const override;
    QStringList extensionHints() const override;
    AbstractAudioFormatIO * getFormatOpen(const QString &filename, QVariant &userData, QWidget *win) override;
    AbstractAudioFormatIO * getFormatLoad(const QString &filename, const QVariant &userData) override;
};

} // talcs

#endif //TALCS_WAVPACKFORMATENTRY_H
