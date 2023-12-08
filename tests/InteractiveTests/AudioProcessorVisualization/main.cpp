/******************************************************************************
 * Copyright (c) 2023 CrSjimo                                                 *
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
#include <QMainWindow>
#include <QPlainTextEdit>
#include <QFile>
#include <QFileDialog>
#include <QThread>

#include <TalcsFormat/AudioFormatIO.h>
#include <TalcsFormat/AudioFormatInputSource.h>

#include "AudioVisualizer.h"

using namespace talcs;

int main(int argc, char **argv) {
    QApplication a(argc, argv);
    QMainWindow win;

    auto area = new QPlainTextEdit;
    area->setReadOnly(true);
    win.setCentralWidget(area);

    auto fileName = QFileDialog::getOpenFileName();
    if (fileName.isEmpty())
        return 0;

    QFile f(fileName);
    AudioFormatIO io(&f);
    AudioFormatInputSource src(&io);
    io.open(QIODevice::ReadOnly);
    auto sampleRate = io.sampleRate();
    src.open(4096, sampleRate);

    AudioVisualizer visualizer(&src, area);
    QThread t;
    visualizer.moveToThread(&t);
    QObject::connect(&t, &QThread::started, &visualizer, &AudioVisualizer::start);
    QObject::connect(&visualizer, &AudioVisualizer::finished, &t, &QThread::quit);
    t.start();

    win.show();
    return a.exec();
}