/******************************************************************************
 * Copyright (c) 2023 CrSjimo                                                 *
 *                                                                            *
 * This file is part of TALCS.                                                *
 *                                                                            *
 * TALCS is free software: you can redistribute it and/or modify it under the *
 * terms of the GNU Lesser General Public License as published by the Free    *
 * Software Foundation, either version 3 of the License, or (at your option)  *
 * any later version.                                                         *
 *                                                                            *
 * TALCS is distributed in the hope that it will be useful, but WITHOUT ANY   *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS  *
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for    *
 * more details.                                                              *
 *                                                                            *
 * You should have received a copy of the GNU Lesser General Public License   *
 * along with TALCS. If not, see <https://www.gnu.org/licenses/>.             *
 ******************************************************************************/

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
    void TransportAudioSourceProcessInfoCallback::onThisBlockProcessInfo(const RemoteProcessInfo &processInfo) {
        if (processInfo.status == RemoteProcessInfo::NotPlaying) {
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