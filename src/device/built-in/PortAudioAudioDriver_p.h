/******************************************************************************
 * Copyright (c) 2023-2025 CrSjimo                                            *
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

#ifndef TALCS_PORTAUDIOAUDIODRIVER_P_H
#define TALCS_PORTAUDIOAUDIODRIVER_P_H

#include <TalcsDevice/private/AudioDriver_p.h>

#include <portaudio.h>

namespace talcs {

    class PortAudioAudioDriverPrivate;
    class PortAudioAudioDevice;

    class PortAudioAudioDriver : public AudioDriver {
        Q_OBJECT
        Q_DECLARE_PRIVATE(PortAudioAudioDriver)
        friend class PortAudioAudioDevice;
    public:
        explicit PortAudioAudioDriver(QObject *parent = nullptr);
        ~PortAudioAudioDriver() override;

        bool initialize() override;
        void finalize() override;
        QStringList devices() const override;
        QString defaultDevice() const override;
        AudioDevice *createDefaultDevice() override;
        AudioDevice *createDevice(const QString &name) override;

        static QList<PortAudioAudioDriver *> getDrivers();
        static bool globalInitialize();
        static void globalFinalize();
    };

    class PortAudioAudioDriverPrivate : public AudioDriverPrivate {
        Q_DECLARE_PUBLIC(PortAudioAudioDriver)
    public:
        PaHostApiIndex hostApiIndex{};
        bool wasapiExclusive{};
        const PaHostApiInfo *hostApiInfo{};
    };

}

#endif // TALCS_PORTAUDIOAUDIODRIVER_P_H
