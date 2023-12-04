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
#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>

using namespace talcs;

int main(int argc, char **argv) {
    QApplication a(argc, argv);
    QFile f("C:\\CloudMusic\\07.恋染色.flac");
    AudioFormatIO io(&f);
    AudioFormatInputSource src(&io);
    BufferingAudioSource bufSrc(&src, 2, 114514);
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
        qDebug() << bufSrc.source()->nextReadPosition() << bufSrc.nextReadPosition();
    });
    timer.start();
    auto win = new QMainWindow;
    auto mainWidget = new QWidget;
    auto mainLayout = new QVBoxLayout;
    mainWidget->setLayout(mainLayout);
    win->setCentralWidget(mainWidget);
    auto resetButton = new QPushButton("Reset Position");
    QObject::connect(resetButton, &QPushButton::clicked, [&] {
        bufSrc.setNextReadPosition(0);
    });
    auto flushButton = new QPushButton("Flush");
    QObject::connect(flushButton, &QPushButton::clicked, [&] {
        bufSrc.flush();
    });
    mainLayout->addWidget(resetButton);
    mainLayout->addWidget(flushButton);
    win->show();
    return a.exec();
}