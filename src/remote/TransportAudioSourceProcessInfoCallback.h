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

#ifndef TALCS_TRANSPORTAUDIOSOURCEPROCESSINFOCALLBACK_H
#define TALCS_TRANSPORTAUDIOSOURCEPROCESSINFOCALLBACK_H

#include <TalcsRemote/RemoteAudioDevice.h>

namespace talcs {

    class TransportAudioSource;

    class TALCSREMOTE_EXPORT TransportAudioSourceProcessInfoCallback : public RemoteAudioDevice::ProcessInfoCallback {
    public:
        explicit TransportAudioSourceProcessInfoCallback(TransportAudioSource *tpSrc);
        void onThisBlockProcessInfo(const RemoteAudioDevice::ProcessInfo &processInfo) override;

    private:
        TransportAudioSource *m_tpSrc;
        bool m_isPaused = true;
    };

}

#endif // TALCS_TRANSPORTAUDIOSOURCEPROCESSINFOCALLBACK_H
