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

#include "ASIOAudioDevice.h"
#include "../AudioDevice_p.h"

#include <qt_windows.h>
#include <combaseapi.h>
#include <iasiodrv.h>

#include <QMutex>

namespace talcs {
    class ASIOAudioDevicePrivate : public AudioDevicePrivate {
        Q_DECLARE_PUBLIC(ASIOAudioDevice);
        IASIO *iasio;
        bool postOutput = false;
        QVector<ASIOBufferInfo> bufferInfoList;
        QVector<ASIOChannelInfo> channelInfoList;
        char errorMessageBuffer[128];

        static void bufferSwitch(long index, ASIOBool processNow);
        static void sampleRateDidChange(ASIOSampleRate sRate);
        static long asioMessage(long selector, long value, void *message, double *opt);
        static ASIOTime *bufferSwitchTimeInfo(ASIOTime *timeInfo, long index, ASIOBool processNow);

        ASIOCallbacks callbacks = {
            &bufferSwitch,
            &sampleRateDidChange,
            &asioMessage,
            &bufferSwitchTimeInfo,
        };

        QMutex mutex;
    };
}

#endif // TALCS_ASIOAUDIODEVICE_P_H
