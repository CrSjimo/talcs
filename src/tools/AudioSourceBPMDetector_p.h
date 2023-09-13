#ifndef TALCS_AUDIOSOURCEBPMDETECTORPRIVATE_H
#define TALCS_AUDIOSOURCEBPMDETECTORPRIVATE_H

#include "AudioSourceBPMDetector.h"

#include <soundtouch/BPMDetect.h>

#include "buffer/AudioBuffer.h"
#include "utils/AudioSourceProcessorBase_p.h"

namespace talcs {

    class AudioSourceBPMDetectorPrivate: public AudioSourceProcessorBasePrivate {
        Q_DECLARE_PUBLIC(AudioSourceBPMDetector);
    public:
        AudioSourceBPMDetectorPrivate(AudioSource *src);
        soundtouch::BPMDetect bpmDetect;
        AudioBuffer buf;
    };

} // talcs

#endif // TALCS_AUDIOSOURCEBPMDETECTORPRIVATE_H
