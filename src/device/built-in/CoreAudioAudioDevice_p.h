// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TALCS_COREAUDIOAUDIODEVICE_P_H
#define TALCS_COREAUDIOAUDIODEVICE_P_H

#include <atomic>

#include <QHash>
#include <QList>
#include <QMutex>
#include <QVector>

#include <AudioUnit/AudioUnit.h>

#include <TalcsCore/AudioBuffer.h>
#include <TalcsCore/AudioDataWrapper.h>
#include <TalcsDevice/private/AudioDevice_p.h>
#include <TalcsDevice/private/CoreAudioAudioDriver_p.h>

QT_BEGIN_NAMESPACE
class QTimer;
QT_END_NAMESPACE

namespace talcs {

    class CoreAudioAudioDevicePrivate;

    class CoreAudioAudioDevice : public AudioDevice {
        Q_OBJECT
        Q_DECLARE_PRIVATE(CoreAudioAudioDevice)
    public:
        CoreAudioAudioDevice(const CoreAudioEndpoint &endpoint, CoreAudioAudioDriver *driver);
        ~CoreAudioAudioDevice() override;

        bool open(qint64 bufferSize, double sampleRate) override;
        void close() override;
        bool start(AudioDeviceCallback *callback) override;
        void stop() override;
        void lock() override;
        void unlock() override;
        bool openControlPanel() override;

        void handleEndpointChange(const QList<CoreAudioEndpoint> &endpoints, const QHash<AudioDeviceID, qint64> &failedDevices);

    private:
        bool updateCapabilities();
        bool openStream(qint64 bufferSize, double sampleRate);
        void handleStreamFailure();
    };

    class CoreAudioAudioDevicePrivate : public AudioDevicePrivate {
        Q_DECLARE_PUBLIC(CoreAudioAudioDevice)
    public:
        CoreAudioEndpoint endpoint;
        CoreAudioAudioDriver::ShareMode mode = CoreAudioAudioDriver::Shared;
        AudioUnit unit = nullptr;
        AudioDeviceCallback *callback = nullptr;
        QMutex callbackMutex;
        QVector<float *> outputPointers;
        AudioDataWrapper outputBuffer{nullptr, 0, 0};
        AudioBuffer buffer;
        qint64 bufferPosition = 0;
        qint64 streamEventTime = 0;
        QTimer *watchdog = nullptr;
        std::atomic<OSStatus> streamError{noErr};
        double originalSampleRate = 0;
        UInt32 originalBufferSize = 0;
        UInt32 hardwareBufferSize = 0;
        bool registered = false;
        bool ownsHog = false;
        bool sampleRateChanged = false;
        bool bufferSizeChanged = false;

        static OSStatus render(void *context, AudioUnitRenderActionFlags *flags, const AudioTimeStamp *time, UInt32 bus, UInt32 frames, AudioBufferList *data);
    };

}

#endif // TALCS_COREAUDIOAUDIODEVICE_P_H
