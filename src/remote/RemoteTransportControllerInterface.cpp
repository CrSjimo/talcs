#include "RemoteTransportControllerInterface.h"
#include "RemoteSocket.h"

namespace talcs {

    /**
     * @class RemoteTransportControllerInterface
     * @brief An interface to control the transport of the remote audio bridge.
     */

    /**
     * Constructor.
     *
     * @note The feature "transport" may not be supported by all remote bridges, and even if the feature is supported,
     * calling these functions may also not achieve the expected results.
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