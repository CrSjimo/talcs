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
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QLabel>
#include <QMainWindow>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QThread>
#include <QVBoxLayout>

#include <TalcsDevice/AudioDevice.h>
#include <TalcsDevice/AudioDriver.h>
#include <TalcsDevice/AudioDriverManager.h>
#include <TalcsDevice/AudioSourcePlayback.h>
#include <TalcsFormat/AudioFormatIO.h>
#include <TalcsFormat/AudioFormatInputSource.h>
#include <TalcsFormat/AudioSourceWriter.h>
#include <TalcsCore/TransportAudioSource.h>

using namespace talcs;

QMainWindow *win;
QLabel *filenameLabel;
QPushButton *startPlayButton;
QPushButton *stopPlayButton;
QPushButton *exportAudioButton;

QFile *inputFile;
AudioFormatIO *inputFormatIO;
AudioFormatInputSource *inputSource;
TransportAudioSource *tpSrc;
AudioSourcePlayback *srcPlayback;

AudioDevice *dev;

void startPlay() {
    if (!dev->start(srcPlayback)) {
        QMessageBox::critical(win, "Error", "Cannot start playing!");
        return;
    }
    tpSrc->play();
    startPlayButton->setDisabled(true);
    stopPlayButton->setDisabled(false);
}

void stopPlay() {
    dev->stop();
    startPlayButton->setDisabled(false);
    stopPlayButton->setDisabled(true);
    exportAudioButton->setDisabled(false);
}

void selectFile() {
    auto filename = QFileDialog::getOpenFileName(win, {}, {}, "*.flac");
    if (filename.isEmpty())
        return;

    stopPlay();
    filenameLabel->setText(filename);
    inputFile->setFileName(filename);
    inputFile->open(QIODevice::ReadOnly);
    tpSrc->setPosition(0);
}

void exportAudio() {
    auto exportFileName = QFileDialog::getSaveFileName(win, {}, {}, "*.wav");
    if (exportFileName.isEmpty())
        return;
    stopPlay();

    QFile exportFile(exportFileName);
    AudioFormatIO exportIO(&exportFile);
    exportFile.open(QIODevice::WriteOnly);
    exportIO.AbstractAudioFormatIO::open(AbstractAudioFormatIO::Write, AudioFormatIO::WAV | AudioFormatIO::PCM_24, 2, 44100);
    tpSrc->open(4096, 44100);
    tpSrc->setPosition(0);
    tpSrc->play();

    QDialog dlg;
    dlg.setWindowTitle("Exporting...");
    auto dlgLayout = new QVBoxLayout;
    auto exportProgressBar = new QProgressBar;
    dlgLayout->addWidget(exportProgressBar);
    dlg.setLayout(dlgLayout);

    QThread thread;
    AudioSourceWriter writer(tpSrc, &exportIO, inputSource->length());
    writer.moveToThread(&thread);
    QObject::connect(&thread, &QThread::started, &writer, &AudioSourceWriter::start);
    QObject::connect(&writer, &AudioSourceWriter::blockProcessed, exportProgressBar, [=](qint64 sampleCountProcessed) {
        exportProgressBar->setValue(sampleCountProcessed * 100 / inputSource->length());
    });
    QObject::connect(&dlg, &QDialog::rejected, &thread, [&]() { writer.interrupt(); });
    QObject::connect(&writer, &AudioSourceWriter::finished, &dlg,[&] {
        if(writer.status() == talcs::AudioSourceProcessorBase::Completed)
            dlg.accept();
        else
            QMessageBox::warning(win, "Export", "Exporting is interrupted.");
    });

    thread.start();
    dlg.exec();
    tpSrc->close();

    thread.quit();
    thread.wait();
}

bool initializeAudioEngine() {
    inputFile = new QFile;
    inputFormatIO = new AudioFormatIO(inputFile);
    inputSource = new AudioFormatInputSource(inputFormatIO);
    tpSrc = new TransportAudioSource(inputSource);
    srcPlayback = new AudioSourcePlayback(tpSrc);

    auto drvMgr = AudioDriverManager::createBuiltInDriverManager();
    auto drv = drvMgr->driver(drvMgr->drivers()[0]);
    if (!drv->initialize()) {
        qCritical() << "Cannot initialize audio driver!";
        return false;
    }
    dev = drv->createDevice(drv->defaultDevice());
    if (!dev || !dev->isInitialized()) {
        qCritical() << "Cannot initialize audio device!";
        return false;
    }
    if (!dev->open(dev->preferredBufferSize(), dev->preferredSampleRate())) {
        qCritical() << "Cannot open audio device!";
        return false;
    }
    return true;
}

int main(int argc, char **argv) {
    QApplication a(argc, argv);

    filenameLabel = new QLabel;
    auto selectFileButton = new QPushButton("Select File");
    startPlayButton = new QPushButton("Start Playing");
    stopPlayButton = new QPushButton("Stop Playing");
    exportAudioButton = new QPushButton("Export Audio");

    startPlayButton->setDisabled(true);
    stopPlayButton->setDisabled(true);
    exportAudioButton->setDisabled(true);

    auto layout = new QVBoxLayout;
    layout->addWidget(filenameLabel);
    layout->addWidget(selectFileButton);
    layout->addWidget(startPlayButton);
    layout->addWidget(stopPlayButton);
    layout->addWidget(exportAudioButton);

    auto centralWidget = new QWidget;
    centralWidget->setLayout(layout);
    win = new QMainWindow;
    win->setCentralWidget(centralWidget);

    QObject::connect(selectFileButton, &QPushButton::clicked, &selectFile);
    QObject::connect(startPlayButton, &QPushButton::clicked, &startPlay);
    QObject::connect(stopPlayButton, &QPushButton::clicked, &stopPlay);
    QObject::connect(exportAudioButton, &QPushButton::clicked, &exportAudio);

    if (!initializeAudioEngine())
        return 1;

    win->show();

    return a.exec();
}
