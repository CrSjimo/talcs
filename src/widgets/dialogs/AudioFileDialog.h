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

#ifndef TALCS_AUDIOFILEDIALOG_H
#define TALCS_AUDIOFILEDIALOG_H

#include <QVariant>
#include <QObject>

#include <TalcsWidgets/TalcsWidgetsGlobal.h>

namespace talcs {

    class AbstractAudioFormatIO;

    class FormatManager;

    class TALCSWIDGETS_EXPORT AudioFileDialog : public QObject {
        Q_OBJECT
    public:
        static AbstractAudioFormatIO *getOpenAudioFileIO(const FormatManager *formatManager,
            QString &fileName, QVariant &userData, QString &entryClassName,
            QWidget *parent = nullptr, const QString &caption = {}, const QString &dir = {});
    };

} // talcs

#endif //TALCS_AUDIOFILEDIALOG_H
