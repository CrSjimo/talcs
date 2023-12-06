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

#ifndef AUDIODEVICE_P_H
#define AUDIODEVICE_P_H

#include <TalcsDevice/AudioDevice.h>

namespace talcs {

    class AudioDevicePrivate {
        Q_DECLARE_PUBLIC(AudioDevice)
    public:
        AudioDevice *q_ptr;
        AudioDriver *driver;
        int channelCount = 0;
        int activeChannelCount = -1;
        QList<qint64> availableBufferSizes;
        qint64 preferredBufferSize = 0;
        QList<double> availableSampleRates;
        double preferredSampleRate = 0;
        bool isStarted = false;
        bool isInitialized = false;
    };
    
}

#endif // AUDIODEVICE_P_H
