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

#ifndef TALCS_OUTPUTCONTEXT_H
#define TALCS_OUTPUTCONTEXT_H

#include <TalcsDevice/AbstractOutputContext.h>

#include <TalcsDevice/AudioDriverManager.h>

namespace talcs {

    class AudioDriverManager;
    class AudioDriver;

    class OutputContextPrivate;

    class TALCSDEVICE_EXPORT OutputContext : public AbstractOutputContext {
        Q_OBJECT
        Q_DECLARE_PRIVATE(OutputContext)
    public:
        explicit OutputContext(QObject *parent = nullptr);
        explicit OutputContext(AudioDriverManager::BuiltInDriverManagerOption option, QObject *parent = nullptr);
        ~OutputContext() override;

        bool initialize(const QString &driverNameHint = {}, const QString &deviceNameHint = {});

        AudioDriverManager *driverManager() const;
        AudioDriver *driver() const;

        enum DeviceOption {
            DO_DefaultOption,
            DO_UsePreferredSpec,
            DO_DoNotChangeAdoptedSpec,
        };
        bool setDriver(const QString &driverName, DeviceOption option = DO_DefaultOption);
        bool setDevice(const QString &deviceName, DeviceOption option = DO_DefaultOption);
        bool enumerateDevices(DeviceOption option = DO_DefaultOption);

        qint64 adoptedBufferSize() const;
        bool setAdoptedBufferSize(qint64 bufferSize);
        double adoptedSampleRate() const;
        bool setAdoptedSampleRate(double sampleRate);

        enum HotPlugNotificationMode {
            Omni,
            Current,
            None,
        };
        Q_ENUM(HotPlugNotificationMode)
        HotPlugNotificationMode hotPlugNotificationMode() const;
        void setHotPlugNotificationMode(HotPlugNotificationMode mode);

    signals:
        void deviceHotPlugged();

    private:
        QScopedPointer<OutputContextPrivate> d_ptr;
    };

} // talcs

#endif //TALCS_OUTPUTCONTEXT_H
