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

#ifndef TALCS_ASIOAUDIODEVICE_P_H
#define TALCS_ASIOAUDIODEVICE_P_H

#include <atomic>

#include <QMutex>
#include <qt_windows.h>

#include <combaseapi.h>

#include <TalcsCore/AudioBuffer.h>
#include <TalcsDevice/private/AudioDevice_p.h>
#include <iasiodrv.h>

QT_BEGIN_NAMESPACE
class QTimer;
QT_END_NAMESPACE

namespace talcs {
    class ASIOAudioDevicePrivate;
    class ASIOAudioDriver;

    class ASIOAudioDevice : public AudioDevice {
        Q_OBJECT
        Q_DECLARE_PRIVATE(ASIOAudioDevice)
    public:
        ~ASIOAudioDevice() override;
        bool start(AudioDeviceCallback *audioDeviceCallback) override;
        void stop() override;
        bool open(qint64 bufferSize, double sampleRate) override;
        void close() override;
        void lock() override;
        void unlock() override;
        bool openControlPanel() override;

    protected:
        friend class ASIOAudioDriver;
        ASIOAudioDevice(const QString &name, const CLSID &clsid, HWND systemHandle, ASIOAudioDriver *driver);

    private:
        void shutdown();
    };
}

namespace talcs {
    class ASIOAudioDevicePrivate : public AudioDevicePrivate {
        Q_DECLARE_PUBLIC(ASIOAudioDevice)
    public:
        enum Notification {
            ResetRequested = 1,
            ResyncRequested = 2,
            SampleRateChanged = 4,
            LatenciesChanged = 8,
            BufferSizeChanged = 16,
        };

        IASIO *iasio = nullptr;
        CLSID clsid{};
        HWND systemHandle = nullptr;
        bool postOutput = false;
        bool buffersCreated = false;
        bool driverRunning = false;
        bool clockLost = false;
        int driverCallDepth = 0;
        qint64 preparedBufferSize = 0;
        qint64 recoveryBufferSize = 0;
        double recoverySampleRate = 0;
        long inputLatency = 0;
        long outputLatency = 0;
        QTimer *notificationTimer = nullptr;
        QTimer *clockRecoveryTimer = nullptr;
        QVector<ASIOBufferInfo> bufferInfoList;
        QVector<ASIOChannelInfo> channelInfoList;
        char errorMessageBuffer[256] = {};

        int deviceIndex = -1;
        ASIOCallbacks callbacks = {};
        AudioDeviceCallback *audioDeviceCallback = nullptr;
        AudioBuffer audioBuffer;

        QRecursiveMutex mutex;

        std::atomic<bool> notificationsEnabled{false};
        std::atomic<quint64> notificationGeneration{0};
        std::atomic<unsigned int> pendingNotifications{0};
        std::atomic<double> notifiedSampleRate{0};
        std::atomic<double> streamingSampleRate{0};
        std::atomic<long> notifiedBufferSize{0};

        void setIASIOError(const char *operation, long result);
        bool initializeInterface();
        void releaseInterface();
        bool updateCapabilities();
        void updateLatencies();
        void requestNotification(unsigned int notification);
        void handleNotifications();
        void checkClockRecovery();
        void render(long index);
    };
}

#endif // TALCS_ASIOAUDIODEVICE_P_H
