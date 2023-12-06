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

#ifndef TALCS_REMOTETRANSPORTCONTROLLERINTERFACE_H
#define TALCS_REMOTETRANSPORTCONTROLLERINTERFACE_H

#include <QtGlobal>

namespace talcs {
    class RemoteSocket;

    class RemoteTransportControllerInterface {
    public:
        explicit RemoteTransportControllerInterface(RemoteSocket *socket);
        ~RemoteTransportControllerInterface() = default;

        void play();
        void pause();

        void setPosition(qint64 position);

        void setLoopingRange(qint64 start, qint64 end);
        void toggleLooping(bool enabled);

    private:
        RemoteSocket *m_socket;
    };

}

#endif // TALCS_REMOTETRANSPORTCONTROLLERINTERFACE_H
