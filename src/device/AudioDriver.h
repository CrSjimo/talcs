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

#ifndef AUDIODRIVER_H
#define AUDIODRIVER_H

#include <QObject>

#include <TalcsCore/ErrorStringProvider.h>
#include <TalcsCore/NameProvider.h>
#include <TalcsDevice/TalcsDeviceGlobal.h>

namespace talcs {

    class AudioDriverPrivate;
    class AudioDevice;

    class TALCSDEVICE_EXPORT AudioDriver : public QObject,
                                           public NameProvider,
                                           public ErrorStringProvider {
        Q_OBJECT
        Q_DECLARE_PRIVATE(AudioDriver)
    public:
        explicit AudioDriver(QObject *parent = nullptr);
        ~AudioDriver() override;

        virtual bool initialize();
        virtual void finalize();
        bool isInitialized() const;

        virtual QStringList devices() const = 0;
        virtual QString defaultDevice() const;
        virtual AudioDevice *createDefaultDevice();
        virtual AudioDevice *createDevice(const QString &name) = 0;

    signals:
        void deviceChanged();

    protected:
        explicit AudioDriver(AudioDriverPrivate &d, QObject *parent);
        QScopedPointer<AudioDriverPrivate> d_ptr;
    };

}

#endif // AUDIODRIVER_H
