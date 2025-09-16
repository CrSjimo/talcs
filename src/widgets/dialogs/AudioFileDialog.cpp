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

#include "AudioFileDialog.h"

#include <algorithm>

#include <QFileDialog>

#include <TalcsFormat/FormatManager.h>
#include <TalcsFormat/FormatEntry.h>

#include <TalcsWidgets/StandardFormatEntry.h>

namespace talcs {
    AbstractAudioFormatIO *AudioFileDialog::getOpenAudioFileIO(const FormatManager *formatManager,
        QString &fileName, QVariant &userData, QString &entryClassName,
        QWidget *parent, const QString &caption, const QString &dir) {
        QStringList extensionList = formatManager->extensionHints();
        std::transform(extensionList.cbegin(), extensionList.cend(), extensionList.begin(), [](const QString &s) {
            return "*." + s;
        });
        auto allSupportedFilesFilter = tr("All Supported Files (%1)").arg(extensionList.join(" "));
        auto rawDataFilter = tr("As Raw Data (*)");
        auto allFilesFilter = tr("All Files (*)");

        auto filters = QStringList{allSupportedFilesFilter} + formatManager->filters() + QStringList{rawDataFilter, allFilesFilter};

        QString selectedFilter;
        fileName = QFileDialog::getOpenFileName(parent, caption, dir, filters.join(";;"), &selectedFilter);
        if (fileName.isEmpty()) {
            return nullptr;
        }
        if (selectedFilter == rawDataFilter) {
            StandardFormatEntry entry;
            userData = QVariantMap({{"raw", true}});
            if (auto io = entry.getFormatOpen(fileName, userData, parent)) {
                entryClassName = StandardFormatEntry::staticMetaObject.className();
                return io;
            }
            return nullptr;
        }
        auto entries = formatManager->entries();
        {
            auto extension = QFileInfo(fileName).suffix();
            auto hintedEntry = extension.isEmpty() ? nullptr : formatManager->hintFromExtension(extension);
            if (hintedEntry) {
                entries.removeOne(hintedEntry);
                entries.prepend(hintedEntry);
            }
        }
        for (auto entry : entries) {
            if (auto io = entry->getFormatOpen(fileName, userData, parent)) {
                entryClassName = entry->metaObject()->className();
                return io;
            }
        }
        return nullptr;
    }
}
