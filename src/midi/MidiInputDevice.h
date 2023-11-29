#ifndef TALCS_MIDIINPUTDEVICE_H
#define TALCS_MIDIINPUTDEVICE_H

#include <TalcsMidi/TalcsMidiGlobal.h>
#include <TalcsCore/NameProvider.h>
#include <TalcsCore/ErrorStringProvider.h>

#include <QObject>

namespace talcs {

    class MidiInputDeviceCallback;

    class MidiInputDevicePrivate;

    class TALCSMIDI_EXPORT MidiInputDevice : public QObject, public NameProvider, public ErrorStringProvider {
        Q_OBJECT
    public:
        explicit MidiInputDevice(int deviceIndex, QObject *parent = nullptr);
        ~MidiInputDevice();

        bool open();
        bool isOpen() const;
        void close();

        void addListener(MidiInputDeviceCallback *callback);
        void removeListener(MidiInputDeviceCallback *callback);

        static QStringList devices();

    private:
        QScopedPointer<MidiInputDevicePrivate> d;
    };

} // talcs

#endif //TALCS_MIDIINPUTDEVICE_H
