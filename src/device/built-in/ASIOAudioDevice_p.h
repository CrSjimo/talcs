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

#ifndef TALCS_ASIOAUDIODEVICE_P_H
#define TALCS_ASIOAUDIODEVICE_P_H

#include <TalcsDevice/ASIOAudioDevice.h>

#include <qt_windows.h>
#include <combaseapi.h>
#include <iasiodrv.h>

#include <QMutex>

#include <TalcsCore/AudioBuffer.h>
#include <TalcsDevice/private/AudioDevice_p.h>

namespace talcs {
    class ASIOAudioDevicePrivate : public AudioDevicePrivate {
        Q_DECLARE_PUBLIC(ASIOAudioDevice);
    public:
        IASIO *iasio;
        bool postOutput = false;
        QVector<ASIOBufferInfo> bufferInfoList;
        QVector<ASIOChannelInfo> channelInfoList;
        char errorMessageBuffer[256] = {};

        int deviceIndex = -1;
        ASIOCallbacks callbacks = {};
        AudioDeviceCallback *audioDeviceCallback = nullptr;
        AudioBuffer audioBuffer;

        QRecursiveMutex mutex;

        void setIASIOError();
    };
}

#endif // TALCS_ASIOAUDIODEVICE_P_H
