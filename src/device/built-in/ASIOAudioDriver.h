#ifndef TALCS_ASIOAUDIODRIVER_H
#define TALCS_ASIOAUDIODRIVER_H

#include "../AudioDriver.h"

namespace talcs {
    class ASIOAudioDriverPrivate;

    class TALCSDEVICE_EXPORT ASIOAudioDriver : public AudioDriver {
        Q_OBJECT
        Q_DECLARE_PRIVATE(ASIOAudioDriver)
    public:
        ASIOAudioDriver(QObject *parent = nullptr);
        ~ASIOAudioDriver() override;
        bool initialize() override;
        void finalize() override;
        QStringList devices() const override;
        QString defaultDevice() const override;
        AudioDevice *createDevice(const QString &name) override;
    };
}

#endif // TALCS_ASIOAUDIODRIVER_H
