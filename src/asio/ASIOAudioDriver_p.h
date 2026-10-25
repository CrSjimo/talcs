/*******************************************************************************
 * Copyright (c) 2023 CrSjimo                                                  *
 * SPDX-License-Identifier: GPL-3.0-only                                       *
 *                                                                             *
 * This file is part of TALCS.                                                 *
 *                                                                             *
 * TALCS is free software: you can redistribute it and/or modify it under the  *
 * terms of the GNU General Public License as published by the Free Software   *
 * Foundation, version 3 of the License.                                       *
 *                                                                             *
 * TALCS is distributed in the hope that it will be useful, but WITHOUT ANY    *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS   *
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more       *
 * details.                                                                    *
 *                                                                             *
 * You should have received a copy of the GNU General Public License along     *
 * with TALCS. If not, see <https://www.gnu.org/licenses/>.                    *
 ******************************************************************************/

#ifndef TALCS_ASIOAUDIODRIVER_P_H
#define TALCS_ASIOAUDIODRIVER_P_H

#include <TalcsAsio/ASIOAudioDriver.h>

#include <qt_windows.h>

#include <TalcsDevice/private/AudioDriver_p.h>

namespace talcs {

    QString asioSystemErrorString(DWORD error);

    struct ASIODeviceSpec {
        CLSID clsid;
        QString driverName;
    };

    class ASIOAudioDriverPrivate : public AudioDriverPrivate {
        Q_DECLARE_PUBLIC(ASIOAudioDriver)
    public:
        QList<ASIODeviceSpec> asioDriverSpecs;
        bool ownsCom = false;
        DWORD comThreadId = 0;
        HWND systemHandle = nullptr;

        void createDriverSpec(HKEY key, const QString &keyName);
    };
}

#endif // TALCS_ASIOAUDIODRIVER_P_H
