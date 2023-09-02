#ifndef TALCS_ASIOAUDIODEVICE_P_H
#define TALCS_ASIOAUDIODEVICE_P_H

#include "ASIOAudioDevice.h"
#include "AudioDevice_p.h"

#include <qt_windows.h>
#include <combaseapi.h>
#include <iasiodrv.h>

#include <QMutex>

namespace talcs {
    class ASIOAudioDevicePrivate : public AudioDevicePrivate {
        Q_DECLARE_PUBLIC(ASIOAudioDevice);
        IASIO *iasio;
        bool postOutput = false;
        QVector<ASIOBufferInfo> bufferInfoList;
        QVector<ASIOChannelInfo> channelInfoList;
        char errorMessageBuffer[128];

        static void bufferSwitch(long index, ASIOBool processNow);
        static void sampleRateDidChange(ASIOSampleRate sRate);
        static long asioMessage(long selector, long value, void *message, double *opt);
        static ASIOTime *bufferSwitchTimeInfo(ASIOTime *timeInfo, long index, ASIOBool processNow);

        ASIOCallbacks callbacks = {
            &bufferSwitch,
            &sampleRateDidChange,
            &asioMessage,
            &bufferSwitchTimeInfo,
        };

        QMutex mutex;
    };
}

#endif // TALCS_ASIOAUDIODEVICE_P_H
