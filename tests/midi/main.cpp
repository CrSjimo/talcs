#include <QApplication>
#include <QDebug>

#include <TalcsMidi/MidiMessage.h>

using namespace talcs;

int main(int argc, char **argv) {
    QApplication a(argc, argv);
    auto msg = MidiMessage::noteOn(1, 60, quint8(100));
    qDebug() << msg.getDescription();
    msg = MidiMessage::midiMachineControlCommand(talcs::MidiMessage::mmc_pause);
    qDebug() << msg.getRawData() << msg.getRawDataSize();
    return a.exec();
}