#include <QApplication>
#include <QFile>
#include <QFileDialog>
#include <QMainWindow>

#include "device/AudioSourcePlayback.h"
#include "format/AudioFormatIO.h"
#include "remote/RemoteAudioDevice.h"
#include "remote/RemoteSocket.h"
#include "source/AudioFormatInputSource.h"
#include "source/SineWaveAudioSource.h"

using namespace talcs;

int main(int argc, char **argv) {
    QApplication a(argc, argv);
    QFile file(QFileDialog::getOpenFileName());
    AudioFormatIO audioFormatIo(&file);
    AudioFormatInputSource src(&audioFormatIo);
    AudioSourcePlayback playback(&src);

    RemoteSocket socket(28081, 28082);
    socket.startServer();
    RemoteAudioDevice device(&socket, {});
    socket.startClient();



    QObject::connect(&socket, &RemoteSocket::socketStatusChanged, [&](RemoteSocket::Status newStatus) {
        qDebug() << "Socket status:" << newStatus;
        if (newStatus != talcs::RemoteSocket::Connected && device.isOpen()) {
            device.close();
        }
    });

    QObject::connect(&device, &RemoteAudioDevice::remoteOpened, &device, [&](){
        device.open(device.preferredBufferSize(), device.preferredSampleRate());
        device.start(&playback);
    });

    return a.exec();
}
