#ifndef TALCS_ASIOAUDIODEVICE_H
#define TALCS_ASIOAUDIODEVICE_H

#include "AudioDevice.h"

struct IASIO;

namespace talcs {
    class ASIOAudioDevicePrivate;
    class ASIOAudioDriver;

    class TALCS_EXPORT ASIOAudioDevice : public AudioDevice {
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
        ASIOAudioDevice(const QString &name, IASIO *iasio, ASIOAudioDriver *driver);
    };
}

#endif // TALCS_ASIOAUDIODEVICE_H
