#include <QApplication>
#include <QFile>
#include <QFileDialog>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QSlider>
#include <QPushButton>

#include "device/AudioSourcePlayback.h"
#include "format/AudioFormatIO.h"
#include "remote/RemoteAudioDevice.h"
#include "remote/RemoteSocket.h"
#include "source/AudioFormatInputSource.h"
#include "source/SineWaveAudioSource.h"
#include "source/TransportAudioSource.h"

using namespace talcs;

int main(int argc, char **argv) {
    QApplication a(argc, argv);
    QFile file(QFileDialog::getOpenFileName());
    AudioFormatIO audioFormatIo(&file);
    AudioFormatInputSource src(&audioFormatIo);
    TransportAudioSource tpSrc(&src);
    AudioSourcePlayback playback(&tpSrc);

    RemoteSocket socket(28081, 28082);
    socket.startServer();
    RemoteAudioDevice device(&socket, {});
    socket.startClient();

    QObject::connect(&socket, &RemoteSocket::socketStatusChanged, [&](int newStatus) {
        qDebug() << "Socket status:" << newStatus;
    });

    QObject::connect(&device, &RemoteAudioDevice::remoteOpened, &device, [&](){
        device.open(device.preferredBufferSize(), device.preferredSampleRate());
        device.start(&playback);
    });

    auto win = new QMainWindow;
    auto mainWidget = new QWidget;
    auto mainLayout = new QVBoxLayout;
    auto slider = new QSlider;
    slider->setOrientation(Qt::Horizontal);
    auto playButton = new QPushButton("Play");
    auto stopButton = new QPushButton("Stop");
    auto buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(playButton);
    buttonLayout->addWidget(stopButton);
    mainLayout->addWidget(slider);
    mainLayout->addLayout(buttonLayout);
    mainWidget->setLayout(mainLayout);
    win->setCentralWidget(mainWidget);
    win->show();

    return a.exec();
}
