#ifndef TALCS_SDLAUDIODEVICE_H
#define TALCS_SDLAUDIODEVICE_H

#include "AudioDevice.h"

namespace talcs {
    class SDLAudioDevicePrivate;
    class SDLAudioDriver;

    class TALCS_EXPORT SDLAudioDevice : public AudioDevice {
        Q_OBJECT
        Q_DECLARE_PRIVATE(SDLAudioDevice)
        friend class SDLAudioDriver;

    public:
        ~SDLAudioDevice() override;
        bool start(AudioDeviceCallback *audioDeviceCallback) override;
        void stop() override;
        bool open(qint64 bufferSize, double sampleRate) override;
        void close() override;
        void lock() override;
        void unlock() override;
        bool openControlPanel() override;

    protected:
        SDLAudioDevice(const QString &name, SDLAudioDriver *driver);
        SDLAudioDevice(SDLAudioDevicePrivate &d, QObject *parent);
    };
}

#endif // TALCS_SDLAUDIODEVICE_H
