#ifndef TALCS_REMOTESOCKET_P_H
#define TALCS_REMOTESOCKET_P_H

#include <TalcsRemote/RemoteSocket.h>

namespace talcs {

    class RemoteSocketPrivate {
    public:
        RemoteSocket *q;

        uint16_t serverPort;
        uint16_t clientPort;
        RemoteSocket::AliveMonitor *aliveMonitor;
        std::atomic<RemoteSocket::Status> status = RemoteSocket::NotConnected;
        QList<QThread *>serverThreads;

        void setStatus(RemoteSocket::Status status);
    };

}

#endif // TALCS_REMOTESOCKET_P_H
