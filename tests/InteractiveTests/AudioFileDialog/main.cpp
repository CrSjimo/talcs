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

#include <QApplication>
#include <QDebug>

#include <TalcsWidgets/AudioFileDialog.h>
#include <TalcsWidgets/StandardFormatEntry.h>
#include <TalcsWidgets/WavpackFormatEntry.h>
#include <TalcsFormat/FormatManager.h>

using namespace talcs;

int main(int argc, char **argv) {
    QApplication a(argc, argv);

    FormatManager fmtMgr;
    fmtMgr.addEntry(new StandardFormatEntry);
    fmtMgr.addEntry(new WavpackFormatEntry);

    QString fileName;
    QVariant userData;
    QString entryClassName;
    auto io = AudioFileDialog::getOpenAudioFileIO(&fmtMgr, fileName, userData, entryClassName);

    qDebug() << fileName << userData << entryClassName << io;

    return a.exec();
}
