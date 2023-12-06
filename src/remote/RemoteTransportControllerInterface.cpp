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

#include "RemoteTransportControllerInterface.h"
#include "RemoteSocket.h"

namespace talcs {

    /**
     * @class RemoteTransportControllerInterface
     * @brief An interface to control the transport of the remote audio bridge.
     * @note The feature "transport" may not be supported by all remote bridges, and even if the feature is supported,
     * calling these functions may also not achieve the expected results.
     */

    /**
     * Constructor.
     *
     * @param socket the remote socket where this object runs
     */
    RemoteTransportControllerInterface::RemoteTransportControllerInterface(RemoteSocket *socket) : m_socket(socket) {
    }

    /**
     * Requests the remote bridge to play.
     */
    void RemoteTransportControllerInterface::play() {
        m_socket->call("transport", "play");
    }

    /**
     * Requests the remote bridge to pause (or stop).
     */
    void RemoteTransportControllerInterface::pause() {
        m_socket->call("transport", "pause");
    }

    /**
     * Requests the remote bridge to change the position of playback.
     * @param position the new position in samples
     */
    void RemoteTransportControllerInterface::setPosition(qint64 position) {
        m_socket->call("transport", "setPosition", position);
    }

    /**
     * Requests the remote bridge to sets the range of loop.
     * @param start the start position in samples
     * @param end the end position in samples
     */
    void RemoteTransportControllerInterface::setLoopingRange(qint64 start, qint64 end) {
        m_socket->call("transport", "setLoopingRange", start, end);
    }

    /**
     * Requests the remote bridge to toggle the state of loop.
     */
    void RemoteTransportControllerInterface::toggleLooping(bool enabled) {
        m_socket->call("transport", "toggleLooping", enabled);
    }
    
}