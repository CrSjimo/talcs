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

#ifndef TALCS_SDLAUDIODRIVER_P_H
#define TALCS_SDLAUDIODRIVER_P_H

#include <TalcsDevice/private/AudioDriver_p.h>

#include <QMap>
#include <QMutex>
#include <QThread>

namespace talcs {

    class SDLAudioDriverPrivate;
    class SDLAudioDevice;

    class SDLAudioDriver : public AudioDriver {
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
        AudioDevice * createDefaultDevice() override;
        AudioDevice *createDevice(const QString &name) override;

        static QList<SDLAudioDriver *> getDrivers();

    protected:
        SDLAudioDriver(SDLAudioDriverPrivate &d, QObject *parent);
        void addOpenedDevice(quint32 devId, SDLAudioDevice *dev);
        void removeOpenedDevice(quint32 devId);
    };

    class SDLEventPoller : public QObject {
        Q_OBJECT
    public:
        explicit SDLEventPoller(QObject *parent = nullptr) : QObject(parent) {}
        QAtomicInteger<bool> stopRequested = false;
    public slots:
        void start();
        void quit();

    signals:
        void event(QByteArray sdlEventData);
    };

    class SDLAudioDriverPrivate : public AudioDriverPrivate {
        Q_DECLARE_PUBLIC(SDLAudioDriver)
    public:
        int driverIndex;
        QScopedPointer<SDLEventPoller> eventPoller;
        QThread *eventPollerThread;
        QMap<quint32, SDLAudioDevice *> openedDevices;
        QString internalName;

        void handleSDLEvent(const QByteArray &sdlEventData);
        void handleDeviceRemoved(quint32 devId);
    };
}

#endif // TALCS_SDLAUDIODRIVER_P_H
