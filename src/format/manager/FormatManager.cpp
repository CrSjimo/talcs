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

#include "FormatManager.h"
#include "FormatManager_p.h"

#include <set>

#include <TalcsFormat/FormatEntry.h>

namespace talcs {
    FormatManager::FormatManager(QObject *parent) : QObject(parent), d_ptr(new FormatManagerPrivate) {
        Q_D(FormatManager);
        d->q_ptr = this;
    }

    FormatManager::~FormatManager() = default;

    void FormatManager::addEntry(FormatEntry *entry) {
        Q_D(FormatManager);
        d->entries.append(entry);
        entry->setParent(this);
        for (const auto &extensionHint : entry->extensionHints()) {
            if (!d->extensionHintDict.contains(extensionHint))
                d->extensionHintDict.insert(extensionHint, entry);
        }
    }

    QList<FormatEntry *> FormatManager::entries() const {
        Q_D(const FormatManager);
        return d->entries;
    }

    QStringList FormatManager::extensionHints() const {
        Q_D(const FormatManager);
        std::set<QString> extensionHintSet;
        for (auto entry : d->entries) {
            auto entryExtensionHints = entry->extensionHints();
            extensionHintSet.insert(entryExtensionHints.cbegin(), entryExtensionHints.cend());
        }
        return {extensionHintSet.cbegin(), extensionHintSet.cend()};
    }

    QStringList FormatManager::filters() const {
        Q_D(const FormatManager);
        QStringList list;
        for (auto entry : d->entries)
            list += entry->filters();
        return list;
    }

    FormatEntry *FormatManager::hintFromExtension(const QString &extension) const {
        Q_D(const FormatManager);
        return d->extensionHintDict.value(extension);
    }

    talcs::AbstractAudioFormatIO *FormatManager::getFormatLoad(const QString &filename, const QVariant &userData) const {
        Q_D(const FormatManager);
        auto extension = (filename + ".").split(".").last();
        auto hintedEntry = extension.isEmpty() ? nullptr : hintFromExtension(extension);
        if (hintedEntry) {
            auto io = hintedEntry->getFormatLoad(filename, userData);
            if (io)
                return io;
        }
        for (auto entry : entries()) {
            if (entry == hintedEntry)
                continue;
            auto io = entry->getFormatLoad(filename, userData);
            if (io)
                return io;
        }
        return nullptr;
    }
}