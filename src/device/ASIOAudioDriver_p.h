#ifndef TALCS_ASIOAUDIODRIVER_P_H
#define TALCS_ASIOAUDIODRIVER_P_H

#include "ASIOAudioDriver.h"
#include "AudioDriver_p.h"

#include <qt_windows.h>

namespace talcs {
    static const int MAXPATHLEN = 512;
    static const int MAXDRVNAMELEN = 128;

    struct ASIODeviceSpec {
        CLSID clsid;
        QString driverName;
    };

    class ASIOAudioDriverPrivate : public AudioDriverPrivate {
        Q_DECLARE_PUBLIC(ASIOAudioDriver)
        QList<ASIODeviceSpec> asioDriverSpecs;
        void createDriverSpec(HKEY hkey, char *keyName);
    };
}



#endif // TALCS_ASIOAUDIODRIVER_P_H
