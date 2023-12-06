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
