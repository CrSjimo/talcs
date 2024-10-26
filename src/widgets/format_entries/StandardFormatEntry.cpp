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

#include "StandardFormatEntry.h"
#include "StandardFormatEntry_p.h"

#include <set>
#include <memory>
#include <limits>

#include <QFile>
#include <QDebug>
#include <QDialog>
#include <QBoxLayout>
#include <QFormLayout>
#include <QSpinBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QPushButton>

#include <TalcsFormat/AudioFormatIO.h>

namespace talcs {
    class AudioFormatIOObject : public QObject, public AudioFormatIO {
    public:
        explicit AudioFormatIOObject(QIODevice *stream = nullptr, QObject *parent = nullptr) : QObject(parent), AudioFormatIO(stream) {

        }
        ~AudioFormatIOObject() override = default;
    };

    StandardFormatEntry::StandardFormatEntry(QObject *parent) : FormatEntry(parent), d_ptr(new StandardFormatEntryPrivate) {
        Q_D(StandardFormatEntry);
        std::set<QString> extensionHintSet;
        for (const auto &fmtInfo : AudioFormatIO::availableFormats()) {
            QStringList fmtExtensions;
            fmtExtensions.append(fmtInfo.extension);
            if (fmtInfo.extension == "raw") {
                for (const auto &subtypeInfo : fmtInfo.subtypes)
                    d->rawSubtypes.append({subtypeInfo.name, subtypeInfo.subtype});
            }
            for (const auto &subtypeInfo : fmtInfo.subtypes) {
                fmtExtensions += subtypeInfo.extensions;
            }
            extensionHintSet.insert(fmtExtensions.cbegin(), fmtExtensions.cend());
            std::transform(fmtExtensions.cbegin(), fmtExtensions.cend(), fmtExtensions.begin(), [](const QString &extension) {
                return "*." + extension;
            });
            d->filters.append(QString("%1 (%2)").arg(tr(fmtInfo.name.toUtf8()), fmtExtensions.join(" ")));
        }
        d->extensionHints = QStringList(extensionHintSet.cbegin(), extensionHintSet.cend());
    }

    StandardFormatEntry::~StandardFormatEntry() = default;

    QStringList StandardFormatEntry::filters() const {
        Q_D(const StandardFormatEntry);
        return d->filters;
    }
    QStringList StandardFormatEntry::extensionHints() const {
        Q_D(const StandardFormatEntry);
        return d->extensionHints;
    }
    AbstractAudioFormatIO *StandardFormatEntry::getFormatOpen(const QString &filename, QVariant &userData, QWidget *win) {
        Q_D(StandardFormatEntry);
        auto io = std::make_unique<AudioFormatIOObject>();
        auto f = std::make_unique<QFile>(filename, io.get());
        if (!f->open(QIODevice::ReadOnly)) {
            qWarning() << "talcs::StandardFormatEntry: Cannot open file on opening" << filename << f->errorString();
            return nullptr;
        }
        io->setStream(f.release());
        if (userData.toMap().value("raw").toBool() || filename.endsWith(".raw")) {
            qDebug() << "talcs::StandardFormatEntry: Opening raw data" << filename;
            QDialog dlg(win);
            auto mainLayout = new QVBoxLayout;
            auto optionsLayout = new QFormLayout;

            auto subtypeComboBox = new QComboBox;
            for (const auto &[name, subtype] : d->rawSubtypes) {
                subtypeComboBox->addItem(name, subtype);
            }
            optionsLayout->addRow(tr("&Option"), subtypeComboBox);

            auto channelCountSpinBox = new QSpinBox;
            channelCountSpinBox->setMinimum(1);
            optionsLayout->addRow(tr("&Channel"), channelCountSpinBox);

            auto sampleRateComboBox = new QComboBox;
            sampleRateComboBox->addItems({"8000", "11025", "12000", "16000", "22050", "24000",
                                          "32000", "44100", "48000", "64000", "88200", "96000",
                                          "128000", "176400", "192000", "256000", "352800",
                                          "384000"});
            sampleRateComboBox->setEditable(true);
            sampleRateComboBox->setValidator(new QDoubleValidator(0.01, std::numeric_limits<double>::max(), 2));
            optionsLayout->addRow(tr("&Sample rate"), sampleRateComboBox);

            auto byteOrderComboBox = new QComboBox;
            byteOrderComboBox->addItem(tr("System"), AudioFormatIO::SystemOrder);
            byteOrderComboBox->addItem(tr("Little-endian"), AudioFormatIO::LittleEndian);
            byteOrderComboBox->addItem(tr("Big-endian"), AudioFormatIO::BigEndian);
            optionsLayout->addRow(tr("&Byte order"), byteOrderComboBox);

            auto offsetSpinBox = new QSpinBox;
            offsetSpinBox->setRange(0, std::numeric_limits<int>::max());
            offsetSpinBox->setDisplayIntegerBase(16);
            offsetSpinBox->setPrefix(QStringLiteral("0x"));
            optionsLayout->addRow(tr("O&ffset"), offsetSpinBox);

            mainLayout->addLayout(optionsLayout);

            auto buttonLayout = new QHBoxLayout;
            buttonLayout->addStretch();
            auto okButton = new QPushButton(tr("OK"));
            okButton->setDefault(true);
            buttonLayout->addWidget(okButton);
            auto cancelButton = new QPushButton(tr("Cancel"));
            buttonLayout->addWidget(cancelButton);
            connect(okButton, &QAbstractButton::clicked, &dlg, &QDialog::accept);
            connect(cancelButton, &QAbstractButton::clicked, &dlg, &QDialog::reject);
            mainLayout->addLayout(buttonLayout);

            dlg.setLayout(mainLayout);
            dlg.setWindowTitle(tr("Configure Raw Data"));
            dlg.setWindowFlag(Qt::WindowContextHelpButtonHint, false);
            if (dlg.exec() != QDialog::Accepted) {
                qWarning() << "StandardFormatEntry: Aborted manually on opening" << filename;
                return nullptr;
            }
            userData = QVariantMap({
                {"raw", true},
                {"subtype", subtypeComboBox->currentData()},
                {"channelCount", channelCountSpinBox->value()},
                {"sampleRate", QLocale().toDouble(sampleRateComboBox->currentText())},
                {"byteOrder", byteOrderComboBox->currentData()},
                {"offset", offsetSpinBox->value()}
            });
            io->setFormat(AudioFormatIO::RAW | subtypeComboBox->currentData().toInt() | byteOrderComboBox->currentData().toInt());
            io->setChannelCount(channelCountSpinBox->value());
            io->setSampleRate(QLocale().toDouble(sampleRateComboBox->currentText()));

        }
        if (!io->open(AbstractAudioFormatIO::Read)) {
            qWarning() << "StandardFormatEntry: Cannot open AudioFormatIO on opening" << filename << io->errorString();
            return nullptr;
        }
        io->close();
        return io.release();
    }
    AbstractAudioFormatIO *StandardFormatEntry::getFormatLoad(const QString &filename, const QVariant &userData) {
        auto io = std::make_unique<AudioFormatIOObject>();
        auto f = std::make_unique<QFile>(filename, io.get());
        if (!f->open(QIODevice::ReadOnly)) {
            qWarning() << "StandardFormatEntry: Cannot open file on loading" << filename << f->errorString();
            return nullptr;
        }
        qint64 streamOffset = 0;
        auto rawOptions = userData.toMap();
        if (rawOptions.value("raw").toBool()) {
            qDebug() << "StandardFormatEntry: Loading raw data" << filename;
            io->setFormat(AudioFormatIO::RAW | rawOptions.value("subtype").toInt() | rawOptions.value("byteOrder").toInt());
            io->setChannelCount(rawOptions.value("channelCount").toInt());
            io->setSampleRate(rawOptions.value("sampleRate").toDouble());
            streamOffset = rawOptions.value("offset").toInt();
        }
        io->setStream(f.release(), streamOffset);
        if (!io->open(AbstractAudioFormatIO::Read)) {
            qWarning() << "StandardFormatEntry: Cannot open AudioFormatIO on loading" << filename << io->errorString();
            return nullptr;
        }
        io->close();
        return io.release();
    }
}