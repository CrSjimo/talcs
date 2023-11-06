#ifndef AUDIODRIVER_H
#define AUDIODRIVER_H

#include <QObject>

#include <TalcsCore/ErrorStringProvider.h>
#include <TalcsCore/NameProvider.h>
#include <TalcsDevice/TalcsDeviceGlobal.h>

namespace talcs {

    class AudioDriverPrivate;
    class AudioDevice;

    class TALCSDEVICE_EXPORT AudioDriver : public QObject,
                                           public NameProvider,
                                           public ErrorStringProvider {
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

#endif // AUDIODRIVER_H