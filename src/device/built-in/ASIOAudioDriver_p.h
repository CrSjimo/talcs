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

#ifndef TALCS_ASIOAUDIODRIVER_P_H
#define TALCS_ASIOAUDIODRIVER_P_H

#include <TalcsDevice/private/AudioDriver_p.h>

#include <qt_windows.h>

namespace talcs {

    class ASIOAudioDriverPrivate;

    class ASIOAudioDriver : public AudioDriver {
        Q_OBJECT
        Q_DECLARE_PRIVATE(ASIOAudioDriver)
    public:
        ASIOAudioDriver(QObject *parent = nullptr);
        ~ASIOAudioDriver() override;
        bool initialize() override;
        void finalize() override;
        QStringList devices() const override;
        QString defaultDevice() const override;
        AudioDevice *createDevice(const QString &name) override;
    };

    static const int MAXPATHLEN = 512;
    static const int MAXDRVNAMELEN = 128;

    struct ASIODeviceSpec {
        CLSID clsid;
        QString driverName;
    };

    class ASIOAudioDriverPrivate : public AudioDriverPrivate {
        Q_DECLARE_PUBLIC(ASIOAudioDriver)
        QList<ASIODeviceSpec> asioDriverSpecs;
        void createDriverSpec(HKEY hkey, char *keyName);
    };
}



#endif // TALCS_ASIOAUDIODRIVER_P_H
