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

#ifndef TALCS_REMOTEPROCESSINFO_H
#define TALCS_REMOTEPROCESSINFO_H

#include <TalcsRemote/TalcsRemoteGlobal.h>

namespace talcs {

    struct RemoteMidiMessage {
        int64_t size;
        int64_t position;
        char data[1];
    };

    struct RemoteMidiMessageList {
        int64_t size;
        RemoteMidiMessage messages[1];
    };

    struct RemoteProcessInfo {
        int containsInfo;

        //== Playback Status Info ==//
        enum PlaybackStatus {
            NotPlaying,
            Playing,
            RealtimePlaying,
        };
        PlaybackStatus status;

        //== Timeline Info ==//
        int timeSignatureNumerator;
        int timeSignatureDenominator;
        double tempo;

        int64_t position;

        //== MIDI ==//
        RemoteMidiMessageList midiMessages;
    };

    class RemoteProcessInfoCallback {
    public:
        virtual void onThisBlockProcessInfo(const RemoteProcessInfo &processInfo) = 0;
    };

}

#endif //TALCS_REMOTEPROCESSINFO_H
