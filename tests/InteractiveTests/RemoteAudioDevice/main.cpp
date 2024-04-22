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
#include <QFile>
#include <QFileDialog>
#include <QLabel>
#include <QMainWindow>
#include <QPushButton>
#include <QSlider>
#include <QVBoxLayout>

#include <TalcsDevice/AudioSourcePlayback.h>
#include <TalcsFormat/AudioFormatIO.h>
#include <TalcsFormat/AudioFormatInputSource.h>
#include <TalcsRemote/RemoteAudioDevice.h>
#include <TalcsRemote/RemoteEditor.h>
#include <TalcsRemote/RemoteSocket.h>
#include <TalcsRemote/TransportAudioSourceProcessInfoCallback.h>
#include <TalcsCore/SineWaveAudioSource.h>
#include <TalcsCore/TransportAudioSource.h>

using namespace talcs;

int main(int argc, char **argv) {
    QApplication a(argc, argv);
    TransportAudioSource tpSrc;
    AudioSourcePlayback playback(&tpSrc);

    //================//

    auto win = new QMainWindow;
    auto mainWidget = new QWidget;
    auto mainLayout = new QVBoxLayout;
    auto slider = new QSlider;
    slider->setOrientation(Qt::Horizontal);
    QObject::connect(slider, &QSlider::valueChanged, &tpSrc, &TransportAudioSource::setPosition);
    QObject::connect(&tpSrc, &TransportAudioSource::positionAboutToChange, slider, [=](int value) {
        QSignalBlocker blocker(slider);
        slider->setValue(value);
    });

    auto playButton = new QPushButton("Play");
    QObject::connect(playButton, &QPushButton::clicked, &tpSrc, &TransportAudioSource::play);
    auto stopButton = new QPushButton("Stop");
    QObject::connect(stopButton, &QPushButton::clicked, &tpSrc, &TransportAudioSource::pause);
    auto buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(playButton);
    buttonLayout->addWidget(stopButton);

    auto fileNameLabel = new QLabel;
    auto browseButton = new QPushButton("Browse");
    auto fileNameLayout = new QHBoxLayout;
    fileNameLayout->addWidget(fileNameLabel);
    fileNameLayout->addWidget(browseButton);

    mainLayout->addLayout(fileNameLayout);
    mainLayout->addWidget(slider);
    mainLayout->addLayout(buttonLayout);
    mainWidget->setLayout(mainLayout);
    win->setCentralWidget(mainWidget);

    //================//

    RemoteSocket socket(28081, 28082);
    socket.startServer();
    RemoteAudioDevice device(&socket, {});
    device.addProcessInfoCallback(new TransportAudioSourceProcessInfoCallback(&tpSrc));

    QObject::connect(&socket, &RemoteSocket::socketStatusChanged, [&](int newStatus) {
        qDebug() << "Socket status:" << newStatus;
    });

    QFile *file = nullptr;
    AudioFormatIO *audioFormatIo = nullptr;
    AudioFormatInputSource *src = nullptr;

    auto openFileFunc = [&](const QString &fileName) {
        AudioDeviceLocker locker(&device);
        if (file) {
            tpSrc.setSource(nullptr);
            delete src;
            delete audioFormatIo;
            delete file;
        }
        file = new QFile(fileName);
        file->open(QIODevice::ReadOnly);
        audioFormatIo = new AudioFormatIO(file);
        src = new AudioFormatInputSource(audioFormatIo);
        tpSrc.setSource(src);
        if (tpSrc.isOpen()) {
            slider->setRange(0, src->length());
        }
        tpSrc.setPosition(0);
        fileNameLabel->setText(fileName);
    };

    RemoteEditor editor(&socket, [=](bool *ok) {
        qDebug() << "Plugin requests to get data";
        *ok = true;
        return fileNameLabel->text().toUtf8();
    }, [=](const QByteArray &data) {
        qDebug() << "Plugin requests to set data";
        openFileFunc(QString::fromUtf8(data));
        return true;
    });

    QObject::connect(browseButton, &QPushButton::clicked, [&] {
        auto fileName = QFileDialog::getOpenFileName();
        if (fileName.isEmpty())
            return;
        openFileFunc(fileName);
        editor.setDirty();
    });

    QObject::connect(&device, &RemoteAudioDevice::remoteOpened, &device, [&](){
        device.open(device.preferredBufferSize(), device.preferredSampleRate());
        device.start(&playback);
        if (src)
            slider->setRange(0, src->length());
    });

    socket.startClient();
    win->show();

    int ret = a.exec();
    socket.stop();
    return ret;
}
