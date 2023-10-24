#ifndef TALCS_AUDIODRIVER_H
#define TALCS_AUDIODRIVER_H

#include <QObject>

#include "core/base/ErrorStringProvider.h"
#include "core/base/NameProvider.h"

namespace talcs {
    class AudioDriverPrivate;
    class AudioDevice;

    class TALCS_EXPORT AudioDriver : public QObject, public NameProvider, public ErrorStringProvider {
        Q_OBJECT
        Q_DECLARE_PRIVATE(AudioDriver)
    public:
        explicit AudioDriver(QObject *parent = nullptr);
        ~AudioDriver() override;

        virtual bool initialize();
        virtual void finalize();
        bool isInitialized() const;

        virtual QStringList devices() const = 0;
        virtual QString defaultDevice() const;
        virtual AudioDevice *createDevice(const QString &name) = 0;

    signals:
        void deviceChanged();

    protected:
        explicit AudioDriver(AudioDriverPrivate &d, QObject *parent);
        QScopedPointer<AudioDriverPrivate> d_ptr;
    };
}

#endif // TALCS_AUDIODRIVER_H
