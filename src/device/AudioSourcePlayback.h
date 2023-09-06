#ifndef AUDIOSOURCEPLAYBACK_H
#define AUDIOSOURCEPLAYBACK_H

#include <QScopedPointer>

#include "AudioDeviceCallback.h"
#include "global/TalcsGlobal.h"

namespace talcs {
    class AudioSourcePlaybackPrivate;
    class AudioSource;

    class TALCS_EXPORT AudioSourcePlayback : public AudioDeviceCallback {
        Q_DECLARE_PRIVATE(AudioSourcePlayback)
    public:
        explicit AudioSourcePlayback(AudioSource *src, bool takeOwnership = false);
        ~AudioSourcePlayback();

        AudioSource *source() const;
        void setSource(AudioSource *src, bool takeOwnership = false);

        void deviceWillStartCallback(AudioDevice *device) override;
        void deviceStoppedCallback() override;
        void workCallback(const AudioSourceReadData &readData) override;

    protected:
        explicit AudioSourcePlayback(AudioSourcePlaybackPrivate &d);
        QScopedPointer<AudioSourcePlaybackPrivate> d_ptr;
    };
}

#endif // AUDIOSOURCEPLAYBACK_H
