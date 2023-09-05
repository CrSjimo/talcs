#ifndef TALCS_SDLAUDIODRIVER_H
#define TALCS_SDLAUDIODRIVER_H

#include "AudioDriver.h"

namespace talcs {
    class SDLAudioDriverPrivate;
    class SDLAudioDriverEventPoller;
    class SDLAudioDevice;

    class TALCS_EXPORT SDLAudioDriver : public AudioDriver {
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
        AudioDevice *createDevice(const QString &name) override;

        static QList<SDLAudioDriver *> getDrivers();

    protected:
        SDLAudioDriver(SDLAudioDriverPrivate &d, QObject *parent);
        void addOpenedDevice(quint32 devId, SDLAudioDevice *dev);
        void removeOpenedDevice(quint32 devId);
    };
}

#endif // TALCS_SDLAUDIODRIVER_H
