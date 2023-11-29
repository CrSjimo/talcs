#ifndef TALCS_MIDISINEWAVESYNTHESIZER_P_H
#define TALCS_MIDISINEWAVESYNTHESIZER_P_H

#include <TalcsMidi/MidiSineWaveSynthesizer.h>
#include <QMutex>

namespace talcs {
    class MidiSineWaveSynthesizerPrivate {
    public:
        QMutex mutex;
        double phase = 0;

        int note = -1;
        int velocity = 0;
        double fadeIn = 0.0;
        double fadeOut = 0.0;
    };
}

#endif //TALCS_MIDISINEWAVESYNTHESIZER_P_H
