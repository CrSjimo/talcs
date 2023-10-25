#include "TransportAudioSourceProcessInfoCallback.h"

#include <TalcsCore/TransportAudioSource.h>

namespace talcs {

    /**
     * @class TransportAudioSourceProcessInfoCallback
     * @brief The TransportAudioSourceProcessInfoCallback object listens for the RemoteAudioDevice::ProcessInfo on each
     * block to be processed and uses it to change the state of a TransportAudioSource object.
     */

    /**
     * Constructor.
     * @param tpSrc the TransportAudioSource object to use
     */
    TransportAudioSourceProcessInfoCallback::TransportAudioSourceProcessInfoCallback(TransportAudioSource *tpSrc) : m_tpSrc(tpSrc) {
    }

    /**
     * The work function handles the following circumstances:
     *   - If the remote audio context is playing and the source is currently playing, the source will be paused.
     *   - If the remote audio context is playing, the source will be played and the position will be synchronized with
     *     the remote audio context.
     */
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
}