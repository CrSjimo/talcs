#ifndef TALCS_REMOTESOCKET_H
#define TALCS_REMOTESOCKET_H

#include <QDebug>
#include <QMutex>
#include <QSet>
#include <QThread>

#include <rpc/client.h>
#include <rpc/server.h>
#include <rpc/rpc_error.h>

#include "global/TalcsGlobal.h"

namespace talcs {

    class RemoteSocketPrivate;

    class TALCS_EXPORT RemoteSocket : public QObject {
        Q_OBJECT
    public:
        class Reply {
        public:
            inline bool isError() const {
                return m_isError;
            }

            inline bool isNull() const {
                return m_isError || m_object.is_nil();
            }

            template <typename T>
            inline T convert() const {
                try {
                    return m_object.as<T>();
                } catch (std::exception &e) {
                    qWarning() << "Exception at RemoteSocket::Reply::convert:" << e.what();
                    return T();
                } catch (...) {
                    return T();
                }
            }

        private:
            friend class RemoteSocket;

            inline explicit Reply(bool isError, clmdep_msgpack::object object) : m_isError(isError), m_object(object) {
            }

            bool m_isError;
            clmdep_msgpack::object m_object;
        };

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

        explicit RemoteSocket(uint16_t serverPort, uint16_t clientPort, QObject *parent = nullptr);
        ~RemoteSocket() override;

        void clientHeartbeat();
        void socketGreet();

        bool startServer();
        bool startClient();
        void stop();

        enum Status {
            NotConnected,
            ServerOnPending,
            ClientOnPending,
            Connected,
        };
        Status status() const;

        template <typename... Args>
        Reply call(const QString &feature, const QString &name, Args... args) {
            QMutexLocker locker(&m_clientMutex);
            if (!m_client || m_client->get_connection_state() != rpc::client::connection_state::connected) {
                return Reply(true, {});
            }
            try {
                auto objRef = m_client->call((feature + "." + name).toStdString(), args...).get();
                return Reply(false, objRef);
            } catch (rpc::rpc_error &e) {
                auto &replyError = e.get_error().get();
                qWarning() << e.what() << (replyError.type == clmdep_msgpack::type::object_type::STR ? replyError.as<std::string>().c_str() : "");
                return Reply(true, {});
            } catch (...) {
                return Reply(true, {});
            }
        }

        template <typename Functor>
        void bind(const QString &feature, const QString &name, Functor f) {
            m_server->bind((feature + "." + name).toStdString(), f);
            m_features[feature]++;
        }

        void unbind(const QString &feature, const QString &name);

        QList<QString> features() const;
        bool hasFeature(const QString &feature) const;

    signals:
        void socketStatusChanged(int newStatus, int oldStatus);

    private:
        QScopedPointer<rpc::server> m_server;
        QScopedPointer<rpc::client> m_client;
        QRecursiveMutex m_clientMutex;
        QMap<QString, int> m_features;

        QScopedPointer<RemoteSocketPrivate> d;
    };

} // talcs

#endif // TALCS_REMOTESOCKET_H
