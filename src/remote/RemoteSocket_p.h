#ifndef TALCS_REMOTESOCKETPRIVATE_H
#define TALCS_REMOTESOCKETPRIVATE_H

#include "RemoteSocket.h"

namespace talcs {

    class RemoteSocketPrivate {
    public:
        RemoteSocket *q;

        uint16_t serverPort;
        uint16_t clientPort;
        RemoteSocket::AliveMonitor *aliveMonitor;
        std::atomic<RemoteSocket::Status> status = RemoteSocket::NotConnected;

        void setStatus(RemoteSocket::Status status);
    };

} // talcs

#endif // TALCS_REMOTESOCKETPRIVATE_H
