#ifndef TALCS_PORTAUDIOAUDIODEVICE_P_H
#define TALCS_PORTAUDIOAUDIODEVICE_P_H

#include <TalcsDevice/private/AudioDevice_p.h>

#include <QMutex>

#include <portaudio.h>

namespace talcs {

    class PortAudioAudioDevicePrivate;
    class PortAudioAudioDriver;

    class PortAudioAudioDevice : public AudioDevice {
        Q_OBJECT
        Q_DECLARE_PRIVATE(PortAudioAudioDevice)
    public:
        ~PortAudioAudioDevice() override;
        bool start(AudioDeviceCallback *audioDeviceCallback) override;
        void stop() override;
        bool open(qint64 bufferSize, double sampleRate) override;
        void close() override;
        void lock() override;
        void unlock() override;
        bool openControlPanel() override;
    protected:
        friend class PortAudioAudioDriver;
        PortAudioAudioDevice(const QString &name, PortAudioAudioDriver *driver);
    };

    class PortAudioAudioDevicePrivate : public AudioDevicePrivate {
        Q_DECLARE_PUBLIC(PortAudioAudioDevice)
    public:
        PaStream *stream = nullptr;
        int deviceIndex = -1;
        int hostApiIndex = -1;
        const PaDeviceInfo *deviceInfo = nullptr;
        AudioDeviceCallback *audioDeviceCallback = nullptr;
        QMutex mutex;
    };

}

#endif // TALCS_PORTAUDIOAUDIODEVICE_P_H
