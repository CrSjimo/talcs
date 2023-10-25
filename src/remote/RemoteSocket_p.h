#ifndef TALCS_REMOTESOCKET_P_H
#define TALCS_REMOTESOCKET_P_H

#include <TalcsRemote/RemoteSocket.h>

namespace talcs {

    class RemoteSocketPrivate {
    public:
        class AliveMonitor : public QThread {
        public:
            AliveMonitor(RemoteSocket *remoteSocket, int intervalMs)
                    : QThread(remoteSocket), m_remoteSocket(remoteSocket), m_intervalMs(intervalMs) {
            }

        protected:
            void run() override;

        private:
            RemoteSocket *m_remoteSocket;
            int m_intervalMs;
        };

        RemoteSocket *q;

        uint16_t serverPort;
        uint16_t clientPort;
        AliveMonitor *aliveMonitor;
        std::atomic<RemoteSocket::Status> status = RemoteSocket::NotConnected;
        QList<QThread *>serverThreads;

        void clientHeartbeat();
        void socketGreet();
        void setStatus(RemoteSocket::Status status);
    };

}

#endif // TALCS_REMOTESOCKET_P_H
