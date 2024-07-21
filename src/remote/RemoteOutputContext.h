/******************************************************************************
 * Copyright (c) 2024 CrSjimo                                                 *
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

#ifndef TALCS_REMOTEOUTPUTCONTEXT_H
#define TALCS_REMOTEOUTPUTCONTEXT_H

#include <TalcsDevice/AbstractOutputContext.h>

#include <TalcsRemote/TalcsRemoteGlobal.h>

namespace talcs {

    class RemoteSocket;
    class RemoteAudioDevice;

    class RemoteOutputContextPrivate;

    class TALCSREMOTE_EXPORT RemoteOutputContext : public AbstractOutputContext {
        Q_OBJECT
        Q_DECLARE_PRIVATE(RemoteOutputContext)
    public:
        explicit RemoteOutputContext(QObject *parent = nullptr);
        ~RemoteOutputContext() override;

        bool initialize(quint16 serverPort, quint16 clientPort);
        bool establishConnection();

        talcs::RemoteSocket *socket() const;
        talcs::RemoteAudioDevice *remoteAudioDevice() const;

    private:
        QScopedPointer<RemoteOutputContextPrivate> d_ptr;
    };

}

#endif //TALCS_REMOTEOUTPUTCONTEXT_H
