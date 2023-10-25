#include <QCoreApplication>
#include <QDebug>
#include <TalcsDevice/AudioDriverManager.h>
#include <TalcsDevice/AudioDriver.h>
#include <TalcsDevice/AudioDevice.h>
#include <TalcsDevice/AudioSourcePlayback.h>
#include <TalcsCore/SineWaveAudioSource.h>

int main(int argc, char **argv) {
    QCoreApplication a(argc, argv);

    auto mgr = talcs::AudioDriverManager::createBuiltInDriverManager();
    qInfo() << "Built-in drivers:" << mgr->drivers();
    auto wasapiDrv = mgr->driver("wasapi");

    if (!wasapiDrv->initialize()) {
        qCritical() << "Cannot initialize WASAPI driver!";
        return 1;
    }
    qInfo() << "WASAPI devices:" << wasapiDrv->devices();
    qInfo() << "WASAPI default device:" << wasapiDrv->defaultDevice();
    auto dev = wasapiDrv->createDevice(wasapiDrv->defaultDevice());

    if (!dev || !dev->isInitialized()) {
        qCritical() << "Cannot create WASAPI audio device!";
        return 1;
    }
    qInfo() << "Preferred buffer size:" << dev->preferredBufferSize();
    qInfo() << "Preferred sample rate:" << dev->preferredSampleRate();
    if (!dev->open(dev->preferredBufferSize(), dev->preferredSampleRate())) {
        qCritical() << "Cannot open WASAPI audio device!";
        return 1;
    }
    auto src = new talcs::SineWaveAudioSource(440);
    auto srcPlayback = new talcs::AudioSourcePlayback(src);
    if (!dev->start(srcPlayback)) {
        qCritical() << "Cannot start playing with WASAPI audio device!";
        return 1;
    }

    a.exec();
}
