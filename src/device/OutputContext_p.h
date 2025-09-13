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

#ifndef TALCS_OUTPUTCONTEXT_P_H
#define TALCS_OUTPUTCONTEXT_P_H

#include <memory>

#include <TalcsDevice/OutputContext.h>

namespace talcs {

    class OutputContextPrivate {
        Q_DECLARE_PUBLIC(OutputContext)
    public:
        OutputContext *q_ptr;

        std::unique_ptr<AudioDriverManager> driverManager;
        AudioDriver *driver = nullptr;
        std::unique_ptr<AudioDevice> device;

        qint64 adoptedBufferSize = 0;
        double adoptedSampleRate = 0.0;

        OutputContext::HotPlugNotificationMode hotPlugNotificationMode = OutputContext::Omni;

        bool openDeviceWithOption(AudioDevice *device, OutputContext::DeviceOption option) const;
        void handleDeviceHotPlug();
        void postSetDevice();

    };
}

#endif //TALCS_OUTPUTCONTEXT_P_H
