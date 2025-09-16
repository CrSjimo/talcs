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

#include <memory>

#include <QApplication>
#include <QFileDialog>
#include <QFile>
#include <QDebug>

#include <TalcsFormat/WavpackAudioFormatIO.h>

using namespace talcs;

int main(int argc, char **argv) {
    QApplication a(argc, argv);

    auto wvFilename = QFileDialog::getOpenFileName();
    if (wvFilename.isEmpty()) {
        qWarning() << "No WV file";
        return 1;
    }
    QFile wvFile(wvFilename);
    if (!wvFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open WV file";
        return 1;
    }
    std::unique_ptr<QFile> wvcFile;
    do {
        auto wvcFilename = QFileDialog::getOpenFileName();
        if (wvcFilename.isEmpty()) {
            qWarning() << "No WVC file";
            break;
        }
        auto scopedWvcFile = std::make_unique<QFile>(wvcFilename);
        if (!scopedWvcFile->open(QIODevice::ReadOnly)) {
            qWarning() << "Cannot open WVC file";
            break;
        }
        wvcFile = std::move(scopedWvcFile);
    } while (false);

    WavpackAudioFormatIO io(&wvFile, wvcFile.get());
    io.setThreadCount(16);

    if (!io.open(AbstractAudioFormatIO::Read)) {
        return 1;
    }

    qDebug() << io.format() << io.channelCount() << io.bitRate() << io.length() << io.sampleRate();

    auto rawFilename = QFileDialog::getSaveFileName();
    if (rawFilename.isEmpty()) {
        qWarning() << "No raw file";
        return 1;
    }
    QFile rawFile(rawFilename);
    if (!rawFile.open(QIODevice::WriteOnly)) {
        qWarning() << "Cannot open raw file";
        return 1;
    }

    QByteArray rawData(io.length() * io.channelCount() * sizeof(float), Qt::Uninitialized);
    io.read(reinterpret_cast<float *>(rawData.data()), io.length());
    rawFile.write(rawData);
    rawFile.close();

    qDebug() << "Finished";

    return a.exec();
}
