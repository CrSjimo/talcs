#include <QApplication>
#include <QDebug>
#include <QTimer>
#include <QFile>

#include <TalcsCore/SineWaveAudioSource.h>
#include <TalcsCore/BufferingAudioSource.h>
#include <TalcsCore/AudioBuffer.h>
#include <TalcsDevice/AudioDriverManager.h>
#include <TalcsDevice/AudioDriver.h>
#include <TalcsDevice/AudioDevice.h>
#include <TalcsDevice/AudioSourcePlayback.h>
#include <TalcsFormat/AudioFormatIO.h>
#include <TalcsFormat/AudioFormatInputSource.h>

using namespace talcs;

int main(int argc, char **argv) {
    QApplication a(argc, argv);
    QFile f("D:\\CloudMusic\\07.恋染色.wav");
    AudioFormatIO io(&f);
    AudioFormatInputSource src(&io);
    BufferingAudioSource bufSrc(&src, 2, 131072);
    auto mgr = AudioDriverManager::createBuiltInDriverManager();
    auto drv = mgr->driver("wasapi");
    drv->initialize();
    auto dev = drv->createDevice(drv->defaultDevice());
    qDebug() << dev->name();
    dev->open(1024, 48000);
    AudioSourcePlayback playback(&bufSrc);
    dev->start(&playback);
    QTimer timer;
    timer.setInterval(1000);
    timer.callOnTimeout([&]{
        qint64 _1 = src.nextReadPosition();
        qint64 _2 = bufSrc.nextReadPosition();
        qDebug() << _1 << _2;
    });
    timer.start();
    return a.exec();
}