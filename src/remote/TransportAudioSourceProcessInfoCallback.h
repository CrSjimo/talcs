#ifndef TALCS_TRANSPORTAUDIOSOURCEPROCESSINFOCALLBACK_H
#define TALCS_TRANSPORTAUDIOSOURCEPROCESSINFOCALLBACK_H

#include "RemoteAudioDevice.h"

namespace talcs {

    class TransportAudioSource;

    class TALCS_EXPORT TransportAudioSourceProcessInfoCallback : public RemoteAudioDevice::ProcessInfoCallback {
    public:
        explicit TransportAudioSourceProcessInfoCallback(TransportAudioSource *tpSrc);
        void onThisBlockProcessInfo(const RemoteAudioDevice::ProcessInfo &processInfo) override;

    private:
        TransportAudioSource *m_tpSrc;
        bool m_isPaused = true;
    };

} // talcs

#endif // TALCS_TRANSPORTAUDIOSOURCEPROCESSINFOCALLBACK_H
