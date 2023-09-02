#ifndef ASIOAUDIODRIVER_H
#define ASIOAUDIODRIVER_H

#include "AudioDriver.h"

namespace talcs {
    class ASIOAudioDriverPrivate;

    class TALCS_EXPORT ASIOAudioDriver : public AudioDriver {
        Q_OBJECT
        Q_DECLARE_PRIVATE(ASIOAudioDriver)
    public:
        ASIOAudioDriver(QObject *parent = nullptr);
        ~ASIOAudioDriver();
        bool initialize() override;
        void finalize() override;
        QStringList devices() const override;
        QString defaultDevice() const override;
        AudioDevice *createDevice(const QString &name) override;

    protected:
        ASIOAudioDriver(ASIOAudioDriverPrivate &d, QObject *parent);
    };
}

#endif // ASIOAUDIODRIVER_H
