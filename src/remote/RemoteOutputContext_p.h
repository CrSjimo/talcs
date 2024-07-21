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

#ifndef TALCS_REMOTEOUTPUTCONTEXT_P_H
#define TALCS_REMOTEOUTPUTCONTEXT_P_H

#include <memory>

#include <TalcsRemote/RemoteOutputContext.h>

namespace talcs {
    class RemoteOutputContextPrivate {
        Q_DECLARE_PUBLIC(RemoteOutputContext)
    public:
        RemoteOutputContext *q_ptr;
        std::unique_ptr<RemoteSocket> socket;
        std::unique_ptr<RemoteAudioDevice> device;

        void handleRemoteDeviceRemoteOpened(qint64 bufferSize, double sampleRate, int maxChannelCount);
    };
}

#endif //TALCS_REMOTEOUTPUTCONTEXT_P_H
