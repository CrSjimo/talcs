#include "TransportAudioSourceProcessInfoCallback.h"

#include "core/source/TransportAudioSource.h"

namespace talcs {
    TransportAudioSourceProcessInfoCallback::TransportAudioSourceProcessInfoCallback(TransportAudioSource *tpSrc) : m_tpSrc(tpSrc) {
    }

    void TransportAudioSourceProcessInfoCallback::onThisBlockProcessInfo(const RemoteAudioDevice::ProcessInfo &processInfo) {
        if (processInfo.status == RemoteAudioDevice::ProcessInfo::NotPlaying) {
            if (m_tpSrc->isPlaying() && !m_isPaused)
                m_tpSrc->pause();
            m_isPaused = true;
        } else {
            if (!m_tpSrc->isPlaying())
                m_tpSrc->play();
            m_isPaused = false;
            if (m_tpSrc->position() != processInfo.position)
                m_tpSrc->setPosition(processInfo.position);
        }
    }
} // talcs