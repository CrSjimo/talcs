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

#ifndef TALCS_REMOTESOCKET_H
#define TALCS_REMOTESOCKET_H

#include <QDebug>
#include <QMutex>
#include <QSet>
#include <QThread>

#include <rpc/client.h>
#include <rpc/server.h>
#include <rpc/rpc_error.h>

#include <TalcsRemote/TalcsRemoteGlobal.h>

namespace talcs {

    class RemoteSocketPrivate;

    class TALCSREMOTE_EXPORT RemoteSocket : public QObject {
        Q_OBJECT
        friend class RemoteSocketPrivate;
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

            inline explicit Reply(bool isError, const clmdep_msgpack::object &object) : m_isError(isError), m_object(object) {
            }

            bool m_isError;
            clmdep_msgpack::object m_object;
        };

        explicit RemoteSocket(uint16_t serverPort, uint16_t clientPort, QObject *parent = nullptr);
        ~RemoteSocket() override;

        bool startServer(int threadCount = 1);
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
                const auto &objRef = m_client->call((feature + "." + name).toStdString(), args...).get();
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

}

#endif // TALCS_REMOTESOCKET_H
