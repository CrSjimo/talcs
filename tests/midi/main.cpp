#include <QApplication>
#include <QDebug>

#include <TalcsMidi/MidiMessage.h>
#include <TalcsMidi/MidiInputDevice.h>
#include <TalcsMidi/MidiSineWaveSynthesizer.h>

#include <TalcsDevice/AudioDriverManager.h>
#include <TalcsDevice/AudioDriver.h>
#include <TalcsDevice/AudioDevice.h>
#include <TalcsDevice/AudioSourcePlayback.h>

using namespace talcs;


int main(int argc, char **argv) {
    QApplication a(argc, argv);
    auto msg = MidiMessage::noteOn(1, 60, quint8(100));
    qDebug() << msg.getDescription();
    msg = MidiMessage::midiMachineControlCommand(talcs::MidiMessage::mmc_pause);
    qDebug() << msg.getRawData() << msg.getRawDataSize();
    qDebug() << MidiInputDevice::devices();
    MidiInputDevice dev(1);

    auto mgr = AudioDriverManager::createBuiltInDriverManager();
    auto drv = mgr->driver(mgr->drivers()[0]);
    drv->initialize();
    auto audioDev = drv->defaultDevice().isEmpty() ? drv->createDevice(drv->devices()[0]) : drv->createDevice(drv->defaultDevice());

    auto midiSynth = new MidiSineWaveSynthesizer;
    auto playback = new AudioSourcePlayback(midiSynth);
    dev.addListener(midiSynth);

    audioDev->open(audioDev->preferredBufferSize(), audioDev->preferredSampleRate());
    audioDev->start(playback);
    dev.open();
    return a.exec();
}