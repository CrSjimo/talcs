#include "RemoteSocket.h"
#include "RemoteSocket_p.h"

#include <QThread>

namespace talcs {

    void RemoteSocket::AliveMonitor::run() {
        for (;;) {
            if (this->isInterruptionRequested())
                break;
            m_remoteSocket->clientHeartbeat();
            QThread::msleep(m_intervalMs);
        }
    }

    RemoteSocket::RemoteSocket(uint16_t serverPort, uint16_t clientPort, QObject *parent)
        : QObject(parent), d(new RemoteSocketPrivate{this, serverPort, clientPort, new AliveMonitor(this, 1000)}) {
    }

    RemoteSocket::~RemoteSocket() {
        RemoteSocket::stop();
    }

    void RemoteSocket::clientHeartbeat() {
        auto connectionState = m_client->get_connection_state();
        Status currentStatus = d->status;
        if (connectionState == rpc::client::connection_state::connected) {
            if (currentStatus == ClientOnPending) {
                if (!call("socket", "greet").isError())
                    d->setStatus(Connected);
            } else if (currentStatus == NotConnected) {
                if (!call("socket", "greet").isError())
                    d->setStatus(ServerOnPending);
            }
        } else {
            if (currentStatus != ClientOnPending)
                d->setStatus(NotConnected);
            if (connectionState != rpc::client::connection_state::initial) {
                QMutexLocker locker(&m_clientMutex);
                m_client.reset(new rpc::client("127.0.0.1", d->clientPort));
                m_client->set_timeout(1000);
            }
        }
    }

    void RemoteSocket::socketGreet() {
        QMutexLocker locker(&m_clientMutex);
        Status currentStatus = d->status;
        if (currentStatus == ServerOnPending)
            d->setStatus(Connected);
        else if (currentStatus == NotConnected)
            d->setStatus(ClientOnPending);
        else if (currentStatus == Connected)
            throw std::runtime_error("Duplicated connection (current status is Connected)");
        else if (currentStatus == ClientOnPending)
            throw std::runtime_error("Duplicated connection (current status is ClientOnPending)");
    }


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
            bind("socket", "greet", [this] { socketGreet(); });
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

    void RemoteSocket::unbind(const QString &feature, const QString &name) {
        /* we temporarily do nothing with the rpc server */
        // m_server->bind((feature + "." + name).toStdString(), &replyUnboundError);
        m_features[feature]--;
        if (m_features[feature] == 0)
            m_features.remove(feature);
    }

    QList<QString> RemoteSocket::features() const {
        return m_features.keys();
    }

    bool RemoteSocket::hasFeature(const QString &feature) const {
        return m_features.contains(feature) != 0;
    }
} // talcs