/******************************************************************************
 * Copyright (c) 2026 CrSjimo                                                 *
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

#include "AACFormatEntry.h"

#include <memory>

#include <QDebug>
#include <QFile>

#ifdef Q_OS_WIN
#  include <TalcsFormat/MediaFoundationAACAudioFormatIO.h>
#endif

namespace talcs {

#ifdef Q_OS_WIN

    namespace {

        class MediaFoundationAACAudioFormatIOObject : public QObject, public MediaFoundationAACAudioFormatIO {
        public:
            explicit MediaFoundationAACAudioFormatIOObject(QObject *parent = nullptr)
                : QObject(parent) {
            }

            ~MediaFoundationAACAudioFormatIOObject() override = default;
        };

        AbstractAudioFormatIO *openAACFile(const QString &filename) {
            auto io = std::make_unique<MediaFoundationAACAudioFormatIOObject>();
            auto file = std::make_unique<QFile>(filename, io.get());
            if (!file->open(QIODevice::ReadOnly)) {
                qWarning() << "AACFormatEntry: Cannot open file" << filename << file->errorString();
                return nullptr;
            }

            io->setStream(file.release());
            if (!io->open(AbstractAudioFormatIO::Read)) {
                qWarning() << "AACFormatEntry: Cannot open MediaFoundationAACAudioFormatIO" << filename
                           << io->errorString();
                return nullptr;
            }
            return io.release();
        }

    }

#endif

    AACFormatEntry::AACFormatEntry(QObject *parent) : FormatEntry(parent) {
    }

    AACFormatEntry::~AACFormatEntry() = default;

    QStringList AACFormatEntry::filters() const {
        static const QStringList filters{"AAC (*.mp4 *.m4a)"};
        return filters;
    }

    QStringList AACFormatEntry::extensionHints() const {
        static const QStringList extensionHints{"mp4", "m4a"};
        return extensionHints;
    }

    AbstractAudioFormatIO *AACFormatEntry::getFormatOpen(const QString &filename, QVariant &, QWidget *) {
#ifdef Q_OS_WIN
        return openAACFile(filename);
#else
        Q_UNUSED(filename)
        return nullptr;
#endif
    }

    AbstractAudioFormatIO *AACFormatEntry::getFormatLoad(const QString &filename, const QVariant &) {
#ifdef Q_OS_WIN
        return openAACFile(filename);
#else
        Q_UNUSED(filename)
        return nullptr;
#endif
    }

}
