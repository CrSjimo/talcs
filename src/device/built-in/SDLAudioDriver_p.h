#ifndef TALCS_SDLAUDIODRIVER_P_H
#define TALCS_SDLAUDIODRIVER_P_H

#include "SDLAudioDriver.h"
#include "../AudioDriver_p.h"

#include <QThread>
#include <QMap>
#include <QMutex>

namespace talcs {
    class SDLEventPoller : public QObject {
        Q_OBJECT
    public:
        explicit SDLEventPoller(QObject *parent = nullptr) : QObject(parent) {}
        QAtomicInteger<bool> stopRequested = false;
    public slots:
        void start();
        void quit();

    signals:
        void event(QByteArray sdlEventData);
    };

    class SDLAudioDriverPrivate : public AudioDriverPrivate {
        Q_DECLARE_PUBLIC(SDLAudioDriver)
    public:
        int driverIndex;
        QScopedPointer<SDLEventPoller> eventPoller;
        QThread *eventPollerThread;
        QMap<quint32, SDLAudioDevice *> openedDevices;

        void handleSDLEvent(const QByteArray &sdlEventData);
        void handleDeviceRemoved(quint32 devId);
    };
}

#endif // TALCS_SDLAUDIODRIVER_P_H
