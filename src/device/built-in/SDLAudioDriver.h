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

#ifndef TALCS_SDLAUDIODRIVER_H
#define TALCS_SDLAUDIODRIVER_H

#include <TalcsDevice/AudioDriver.h>

namespace talcs {
    class SDLAudioDriverPrivate;
    class SDLAudioDriverEventPoller;
    class SDLAudioDevice;

    class TALCSDEVICE_EXPORT SDLAudioDriver : public AudioDriver {
        Q_OBJECT
        Q_DECLARE_PRIVATE(SDLAudioDriver)
        friend class SDLAudioDevice;

    public:
        explicit SDLAudioDriver(QObject *parent = nullptr);
        ~SDLAudioDriver() override;

        bool initialize() override;
        void finalize() override;
        QStringList devices() const override;
        QString defaultDevice() const override;
        AudioDevice *createDevice(const QString &name) override;

        static QList<SDLAudioDriver *> getDrivers();

    protected:
        SDLAudioDriver(SDLAudioDriverPrivate &d, QObject *parent);
        void addOpenedDevice(quint32 devId, SDLAudioDevice *dev);
        void removeOpenedDevice(quint32 devId);
    };
}

#endif // TALCS_SDLAUDIODRIVER_H
