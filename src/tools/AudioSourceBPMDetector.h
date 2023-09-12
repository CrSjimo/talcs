#ifndef TALCS_AUDIOSOURCEBPMDETECTOR_H
#define TALCS_AUDIOSOURCEBPMDETECTOR_H

#include "utils/AudioSourceProcessorBase.h"

namespace talcs {

    class AudioSourceBPMDetectorPrivate;

    class TALCS_EXPORT AudioSourceBPMDetector: public AudioSourceProcessorBase {
        Q_OBJECT
        Q_DECLARE_PRIVATE(AudioSourceBPMDetector)
    public:
        AudioSourceBPMDetector(AudioSource *src, qint64 length, QObject *parent = nullptr);
        ~AudioSourceBPMDetector() override;

        float bpm() const;

    protected:
        IAudioSampleContainer *prepareBuffer() override;
        bool processBlock(qint64 processedSampleCount, qint64 samplesToProcess) override;
        void processWillFinish() override;
    };

} // talcs

#endif // TALCS_AUDIOSOURCEBPMDETECTOR_H
