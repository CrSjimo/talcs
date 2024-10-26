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

#include "WavpackFormatEntry.h"

#include <memory>
#include <tuple>

#include <QFile>
#include <QDebug>
#include <QFileDialog>
#include <QCheckBox>
#include <QDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

#include <TalcsFormat/WavpackAudioFormatIO.h>


namespace talcs {

    class WavpackAudioFormatIOObject : public QObject, public WavpackAudioFormatIO {
    public:
        explicit WavpackAudioFormatIOObject(QFileDevice *stream, QFileDevice *corrStream, QObject *parent = nullptr) : QObject(parent), WavpackAudioFormatIO(stream, corrStream) {

        }
        ~WavpackAudioFormatIOObject() override = default;
    };

    WavpackFormatEntry::WavpackFormatEntry(QObject *parent) : FormatEntry(parent) {
    }

    WavpackFormatEntry::~WavpackFormatEntry() = default;

    QStringList WavpackFormatEntry::filters() const {
        static const QStringList filters {"Wavpack (*.wv)"};
        return filters;
    }

    QStringList WavpackFormatEntry::extensionHints() const {
        static const QStringList extensionHints {"wv"};
        return extensionHints;
    }

    AbstractAudioFormatIO * WavpackFormatEntry::getFormatOpen(const QString &filename, QVariant &userData, QWidget *win) {
        auto f = std::make_unique<QFile>(filename);
        if (!f->open(QIODevice::ReadOnly)) {
            qWarning() << "WavpackFormatEntry: Cannot open file on opening" << filename;
            return nullptr;
        }
        auto io = std::make_unique<WavpackAudioFormatIOObject>(f.get(), nullptr);
        if (!io->open(AbstractAudioFormatIO::Read)) {
            qWarning() << "WavpackFormatEntry: Cannot open WavpackAudioFormatIO (w/o correction file) on opening" << filename << io->errorString();
            return nullptr;
        }
        if (qIsNaN(io->bitRate())) {
            qDebug() << "WavpackFormatEntry: lossless file" << filename;
            f->setParent(io.get());
            std::ignore = f.release();
            return io.release();
        }

        QDialog dlg(win);
        auto mainLayout = new QVBoxLayout;

        auto browseLayout = new QHBoxLayout;
        auto filenameLineEdit = new QLineEdit;
        filenameLineEdit->setReadOnly(true);
        browseLayout->addWidget(filenameLineEdit);
        auto browseButton = new QPushButton(tr("&Browse..."));
        browseLayout->addWidget(browseButton);
        mainLayout->addLayout(browseLayout);

        auto buttonLayout = new QHBoxLayout;
        buttonLayout->addStretch();
        auto okButton = new QPushButton(tr("OK"));
        okButton->setDisabled(true);
        okButton->setDefault(true);
        buttonLayout->addWidget(okButton);
        auto skipButton = new QPushButton(tr("Skip"));
        buttonLayout->addWidget(skipButton);
        mainLayout->addLayout(buttonLayout);

        dlg.setLayout(mainLayout);
        dlg.setWindowFlag(Qt::WindowContextHelpButtonHint, false);
        dlg.setWindowTitle(tr("Open Wavpack Correction File"));

        connect(browseButton, &QAbstractButton::clicked, &dlg, [=, &dlg] {
            auto corrFilename = QFileDialog::getOpenFileName(&dlg, {}, QFileInfo(filename).dir().path(), "*.wvc");
            if (corrFilename.isEmpty())
                return;
            filenameLineEdit->setText(corrFilename);
            okButton->setDisabled(false);
        });

        connect(okButton, &QAbstractButton::clicked, &dlg, &QDialog::accept);
        connect(skipButton, &QAbstractButton::clicked, &dlg, &QDialog::reject);

        if (QFileInfo(filename).suffix() == "wv") {
            auto defaultCorrFilename = filename + "c";
            if (QFileInfo(defaultCorrFilename).isFile()) {
                qDebug() << "WavpackFormatEntry: default correction file found" << defaultCorrFilename;
                filenameLineEdit->setText(defaultCorrFilename);
                okButton->setDisabled(false);
            }
        }

        while (true) {
            if (dlg.exec() == QDialog::Rejected) {
                f->setParent(io.get());
                std::ignore = f.release();
                return io.release();
            }
            auto corrFile = std::make_unique<QFile>(filenameLineEdit->text());
            auto corrIO = std::make_unique<WavpackAudioFormatIOObject>(f.get(), corrFile.get());
            do {
                if (!corrFile->open(QIODevice::ReadOnly)) {
                    qWarning() << "WavpackFormatEntry: Cannot open correction file on opening" << corrFile->fileName();
                    break;
                }
                if (!corrIO->open(AbstractAudioFormatIO::Read)) {
                    qWarning() << "WavpackFormatEntry: Cannot open corrected WavpackAudioFormatIO on opening" << filename << corrFile->fileName() << io->errorString();
                    break;
                }
                userData = QVariantMap({
                    {"wvc", corrFile->fileName()},
                });
                f->setParent(corrIO.get());
                corrFile->setParent(corrIO.get());
                std::ignore = f.release();
                std::ignore = corrFile.release();
                qDebug() << corrIO.get();
                return corrIO.release();
            } while (false);
            switch(QMessageBox::critical(win, {}, tr("Cannot open Wavpack correction file:\n%1\n\nPress \"Retry\" to reselect a correction file\nPress \"Ignore\" to continue without correction file").arg(corrFile->fileName()), QMessageBox::Retry | QMessageBox::Ignore | QMessageBox::Cancel)) {
                case QMessageBox::Ignore:
                    f->setParent(io.get());
                    std::ignore = f.release();
                    return io.release();
                case QMessageBox::Cancel:
                    return nullptr;
                default:
                    continue;
            }
        }
    }

    AbstractAudioFormatIO * WavpackFormatEntry::getFormatLoad(const QString &filename, const QVariant &userData) {
        auto f = std::make_unique<QFile>(filename);
        if (!f->open(QIODevice::ReadOnly)) {
            qWarning() << "WavpackFormatEntry: Cannot open file on loading" << filename;
            return nullptr;
        }
        std::unique_ptr<QFile> corrFile;
        auto corrFilename = userData.toMap().value("wvc").toString();
        if (!corrFilename.isEmpty()) {
            corrFile = std::make_unique<QFile>(corrFilename);
            if (!corrFile->open(QIODevice::ReadOnly)) {
                qWarning() << "WavpackFormatEntry: Cannot open correction file on loading" << filename << corrFile->fileName();
                return nullptr;
            }
        }
        auto io = std::make_unique<WavpackAudioFormatIOObject>(f.get(), corrFile.get());
        if (!io->open(AbstractAudioFormatIO::Read)) {
            qWarning() << "WavpackFormatEntry: Cannot open WavpackAudioFormatIO on loading" << filename << (corrFile ? corrFile->fileName() : "") << io->errorString();
            return nullptr;
        }
        f->setParent(io.get());
        corrFile->setParent(io.get());
        std::ignore = f.release();
        std::ignore = corrFile.release();
        return io.release();
    }

}