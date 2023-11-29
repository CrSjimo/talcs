#ifndef TALCS_MIDISINEWAVESYNTHESIZER_H
#define TALCS_MIDISINEWAVESYNTHESIZER_H

#include <TalcsCore/AudioSource.h>
#include <TalcsMidi/MidiInputDeviceCallback.h>

namespace talcs {

    class MidiSineWaveSynthesizerPrivate;

    class TALCSMIDI_EXPORT MidiSineWaveSynthesizer : public AudioSource, public MidiInputDeviceCallback {
    public:
        MidiSineWaveSynthesizer();

        bool open(qint64 bufferSize, double sampleRate) override;

        void close() override;

        ~MidiSineWaveSynthesizer() override;

        qint64 read(const AudioSourceReadData &readData) override;

        void deviceWillStartCallback(MidiInputDevice *device) override;

        void deviceStoppedCallback() override;

        void workCallback(const MidiMessage &message) override;

        void errorCallback(const QString &errorString) override;

    private:
        QScopedPointer<MidiSineWaveSynthesizerPrivate> d;
    };

} // talcs

#endif //TALCS_MIDISINEWAVESYNTHESIZER_H
