#include "RemoteTransportControllerInterface.h"
#include "RemoteSocket.h"

namespace talcs {
    RemoteTransportControllerInterface::RemoteTransportControllerInterface(RemoteSocket *socket) : m_socket(socket) {
    }

    void RemoteTransportControllerInterface::play() {
        m_socket->call("transport", "play");
    }

    void RemoteTransportControllerInterface::pause() {
        m_socket->call("transport", "pause");
    }

    void RemoteTransportControllerInterface::setPosition(qint64 position) {
        m_socket->call("transport", "setPosition", position);
    }

    void RemoteTransportControllerInterface::setLoopingRange(qint64 start, qint64 end) {
        m_socket->call("transport", "setLoopingRange", start, end);
    }

    void RemoteTransportControllerInterface::toggleLooping(bool enabled) {
        m_socket->call("transport", "toggleLooping", enabled);
    }
} // talcs