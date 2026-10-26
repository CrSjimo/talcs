// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TALCS_COREAUDIOAUDIODRIVER_P_H
#define TALCS_COREAUDIOAUDIODRIVER_P_H

#include <sys/types.h>

#include <QHash>
#include <QList>
#include <QString>

#include <CoreAudio/CoreAudio.h>

#include <TalcsDevice/private/AudioDriver_p.h>

QT_BEGIN_NAMESPACE
class QTimer;
QT_END_NAMESPACE

namespace talcs {

    struct CoreAudioEndpoint {
        AudioDeviceID id = kAudioObjectUnknown;
        QString uid;
        QString name;
        int channels = 0;
        double sampleRate = 0;
        UInt32 bufferSize = 0;
        pid_t hogOwner = -1;
    };

    class CoreAudioAudioDriverPrivate;

    class CoreAudioAudioDriver : public AudioDriver {
        Q_OBJECT
        Q_DECLARE_PRIVATE(CoreAudioAudioDriver)
    public:
        enum ShareMode {
            Shared,
            Hog,
        };

        explicit CoreAudioAudioDriver(ShareMode mode, QObject *parent = nullptr);
        ~CoreAudioAudioDriver() override;

        bool initialize() override;
        void finalize() override;
        QStringList devices() const override;
        QString defaultDevice() const override;
        AudioDevice *createDevice(const QString &name) override;

        ShareMode shareMode() const;
        void requestRefresh();

    private:
        bool refreshDevices(bool notifyChanges = true);
        static OSStatus propertyChanged(AudioObjectID object, UInt32 count, const AudioObjectPropertyAddress addresses[], void *context);
    };

    struct CoreAudioPropertyListener {
        AudioObjectID object;
        AudioObjectPropertyAddress address;
    };

    class CoreAudioAudioDriverPrivate : public AudioDriverPrivate {
        Q_DECLARE_PUBLIC(CoreAudioAudioDriver)
    public:
        CoreAudioAudioDriver::ShareMode mode = CoreAudioAudioDriver::Shared;
        QTimer *refreshTimer = nullptr;
        QList<CoreAudioEndpoint> endpoints;
        QList<CoreAudioPropertyListener> listeners;
        QHash<AudioDeviceID, qint64> failedDevices;
    };

}

#endif // TALCS_COREAUDIOAUDIODRIVER_P_H
