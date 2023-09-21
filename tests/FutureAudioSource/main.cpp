//
// Created by Crs_1 on 2023/9/21.
//

#include <QCoreApplication>
#include <QFuture>
#include <QThread>

#include <QFutureWatcher>
#include <device/AudioDevice.h>
#include <device/AudioDriver.h>
#include <device/AudioDriverManager.h>
#include <device/AudioSourcePlayback.h>
#include <source/SineWaveAudioSource.h>
#include <synthesis/FutureAudioSource.h>

using namespace talcs;

void work(QFutureInterface<PositionableAudioSource *> *d, PositionableAudioSource *src) {
    for(int i = 0; i < 100; i++) {
        QThread::msleep(100);
        d->setProgressValue(i);
    }
    d->reportFinished(&src);
}

int main(int argc, char **argv) {
    QCoreApplication a(argc, argv);
    auto mgr = AudioDriverManager::createBuiltInDriverManager();
    auto wasapiDrv = mgr->driver(mgr->drivers()[0]);
    wasapiDrv->initialize();
    auto dev = wasapiDrv->createDevice(wasapiDrv->defaultDevice());
    dev->open(1024, 48000);

    auto src = new SineWaveAudioSource(440);

    QFutureInterface<PositionableAudioSource *> futureInterface;
    futureInterface.reportStarted();
    auto future = futureInterface.future();

    auto futureSrc = new FutureAudioSource(future);

    auto t = QThread::create(&work, &futureInterface, src);
    auto srcPlayback = new AudioSourcePlayback(futureSrc);
    dev->start(srcPlayback);
    QObject::connect(futureSrc, &FutureAudioSource::progressChanged, [](int progress){
        qDebug() << "Progress: " << progress;
    });
    t->start();
    return a.exec();
}