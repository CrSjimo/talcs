#ifndef TALCS_PORTAUDIOAUDIODRIVER_P_H
#define TALCS_PORTAUDIOAUDIODRIVER_P_H

#include <TalcsDevice/private/AudioDriver_p.h>

#include <portaudio.h>

namespace talcs {

    class PortAudioAudioDriverPrivate;
    class PortAudioAudioDevice;

    class PortAudioAudioDriver : public AudioDriver {
        Q_OBJECT
        Q_DECLARE_PRIVATE(PortAudioAudioDriver)
        friend class PortAudioAudioDevice;
    public:
        explicit PortAudioAudioDriver(QObject *parent = nullptr);
        ~PortAudioAudioDriver() override;

        bool initialize() override;
        void finalize() override;
        QStringList devices() const override;
        QString defaultDevice() const override;
        AudioDevice *createDefaultDevice() override;
        AudioDevice *createDevice(const QString &name) override;

        static QList<PortAudioAudioDriver *> getDrivers();
        static bool globalInitialize();
        static void globalFinalize();
    };

    class PortAudioAudioDriverPrivate : public AudioDriverPrivate {
        Q_DECLARE_PUBLIC(PortAudioAudioDriver)
    public:
        PaHostApiIndex hostApiIndex{};
        bool wasapiExclusive{};
        const PaHostApiInfo *hostApiInfo{};
    };

}

#endif // TALCS_PORTAUDIOAUDIODRIVER_P_H
