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

#include "RemoteSocket.h"
#include "RemoteSocket_p.h"

#include <QThread>

namespace talcs {

    void RemoteSocketPrivate::AliveMonitor::run() {
        for (;;) {
            if (this->isInterruptionRequested())
                break;
            m_remoteSocket->d->clientHeartbeat();
            QThread::msleep(m_intervalMs);
        }
    }

    /**
     * @class RemoteSocket
     * @brief A RemoteSocket object provides a duplex connection for RPC.
     */

    /**
     * Constructor.
     * @param serverPort the TCP port of server to listen
     * @param clientPort the TCP port of client to connect
     */
    RemoteSocket::RemoteSocket(uint16_t serverPort, uint16_t clientPort, QObject *parent)
        : QObject(parent), d(new RemoteSocketPrivate{this, serverPort, clientPort, new RemoteSocketPrivate::AliveMonitor(this, 1000)}) {
    }

    /**
     * Destructor.
     *
     * Note that if the socket is still connected, it will be closed now.
     */
    RemoteSocket::~RemoteSocket() {
        RemoteSocket::stop();
    }

    void RemoteSocketPrivate::clientHeartbeat() {
        auto connectionState = q->m_client->get_connection_state();
        RemoteSocket::Status currentStatus = status;
        if (connectionState == rpc::client::connection_state::connected) {
            if (currentStatus == RemoteSocket::ClientOnPending) {
                if (!q->call("socket", "greet").isError())
                    setStatus(RemoteSocket::Connected);
            } else if (currentStatus == RemoteSocket::NotConnected) {
                if (!q->call("socket", "greet").isError())
                    setStatus(RemoteSocket::ServerOnPending);
            }
        } else {
            if (currentStatus != RemoteSocket::ClientOnPending)
                setStatus(RemoteSocket::NotConnected);
            if (connectionState != rpc::client::connection_state::initial) {
                QMutexLocker locker(&q->m_clientMutex);
                q->m_client.reset(new rpc::client("127.0.0.1", clientPort));
                q->m_client->set_timeout(1000);
            }
        }
    }

    void RemoteSocketPrivate::socketGreet() {
        QMutexLocker locker(&q->m_clientMutex);
        RemoteSocket::Status currentStatus = status;
        if (currentStatus == RemoteSocket::ServerOnPending)
            setStatus(RemoteSocket::Connected);
        else if (currentStatus == RemoteSocket::NotConnected)
            setStatus(RemoteSocket::ClientOnPending);
        else if (currentStatus == RemoteSocket::Connected)
            throw std::runtime_error("Duplicated connection (current status is Connected)");
        else if (currentStatus == RemoteSocket::ClientOnPending)
            throw std::runtime_error("Duplicated connection (current status is ClientOnPending)");
    }


    /**
     * Starts the socket server.
     * @param threadCount the number of server dispatcher thread
     * @return @c true if successful
     */
    bool RemoteSocket::startServer(int threadCount) {
        try {
            m_server.reset(new rpc::server("127.0.0.1", d->serverPort));
            m_server->suppress_exceptions(true);
            bind("socket", "hasFeature", [=](const std::string &feature) { return hasFeature(feature.c_str()); });
            bind("socket", "features", [=]() {
                std::vector<std::string> vec;
                for (const QString &feature : m_features.keys()) {
                    vec.emplace_back(feature.toStdString());
                }
                return vec;
            });
            bind("socket", "greet", [this] { d->socketGreet(); });
            for (int i = 0; i < threadCount; i++) {
                auto t = QThread::create([=] { m_server->run(); });
                d->serverThreads.append(t);
                t->start(QThread::TimeCriticalPriority);
            }
        } catch (std::exception &e) {
            qWarning() << "Exception at RemoteSocket::startServer:" << e.what();
            return false;
        } catch (...) {
            return false;
        }
        return true;
    }

    /**
     * Starts the socket client.
     *
     * Note that all bindings should be done before client starts.
     * @return @c true if successful
     */
    bool RemoteSocket::startClient() {
        try {
            m_client.reset(new rpc::client("127.0.0.1", d->clientPort));
            m_client->set_timeout(1000);
            d->aliveMonitor->start();
        } catch (std::exception &e) {
            qWarning() << "Exception at RemoteSocket::startClient:" << e.what();
            return false;
        } catch (...) {
            return false;
        }
        return true;
    }

    /**
     * Stops the server and the client.
     */
    void RemoteSocket::stop() {
        QMutexLocker locker(&m_clientMutex);
        d->aliveMonitor->requestInterruption();
        d->aliveMonitor->quit();
        d->aliveMonitor->wait();
        if (m_server) {
            m_server->stop();
            for (auto t: d->serverThreads) {
                t->wait();
                delete t;
            }
            d->serverThreads.clear();
        }
        m_server.reset();
        m_client.reset();
    }

    /**
     * @enum RemoteSocket::Status
     * Socket status.
     *
     * @var RemoteSocket::NotConnected
     * Both server and client are not connected.
     *
     * @var RemoteSocket::ServerOnPending
     * The client is connected to the remote server, but the server is still waiting for remote client connection.
     *
     * @var RemoteSocket::ClientOnPending.
     * The remote client is connected to the server, but the client is still trying to connect to the remote server.
     *
     * @var RemoteSocket::Connected
     * Both server and client is connected.
     */

    /**
     * Gets the status.
     */
    RemoteSocket::Status RemoteSocket::status() const {
        return d->status;
    }

    void RemoteSocketPrivate::setStatus(RemoteSocket::Status status_) {
        RemoteSocket::Status oldStatus = status;
        if (oldStatus != status_) {
            emit q->socketStatusChanged(status_, oldStatus);
            status = status_;
        }
    }

    static void replyUnboundError() {
        throw std::runtime_error("Target function unbound");
    }

    /**
     * @fn void RemoteSocket::Reply RemoteSocket::call(const QString &feature, const QString &name, Args... args)
     * Calls a remote function.
     */

    /**
     * @fn void RemoteSocket::bind(const QString &feature, const QString &name, Functor f)
     * Binds a function.
     */

    /**
     * Unbinds a function.
     *
     * Note that the same function should not be bound again after unbound, until the socket is stopped and restarted.
     */
    void RemoteSocket::unbind(const QString &feature, const QString &name) {
        /* we temporarily do nothing with the rpc server */
        // m_server->bind((feature + "." + name).toStdString(), &replyUnboundError);
        m_features[feature]--;
        if (m_features[feature] == 0)
            m_features.remove(feature);
    }

    /**
     * Gets all available features.
     */
    QList<QString> RemoteSocket::features() const {
        return m_features.keys();
    }

    /**
     * Checks whether a feature is available.
     */
    bool RemoteSocket::hasFeature(const QString &feature) const {
        return m_features.contains(feature) != 0;
    }

    /**
     * @fn void RemoteSocket::socketStatusChanged(int newStatus, int oldStatus)
     * Emitted when the socket status is changed.
     */
    
}