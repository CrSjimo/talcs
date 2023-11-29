#ifndef TALCS_MIDIINPUTDEVICE_H
#define TALCS_MIDIINPUTDEVICE_H

#include <TalcsMidi/TalcsMidiGlobal.h>
#include <TalcsCore/NameProvider.h>
#include <TalcsCore/ErrorStringProvider.h>

#include <QObject>

namespace talcs {

    class MidiInputDeviceCallback;

    class MidiInputDevicePrivate;

    class MidiInputDevice : public QObject, public NameProvider, public ErrorStringProvider {
        Q_OBJECT
    public:
        explicit MidiInputDevice(int deviceIndex, QObject *parent = nullptr);

        bool open(MidiInputDeviceCallback *callback);
        bool isOpen() const;
        void close();

        static QStringList devices();

    private:
        QScopedPointer<MidiInputDevicePrivate> d;
    };

} // talcs

#endif //TALCS_MIDIINPUTDEVICE_H
