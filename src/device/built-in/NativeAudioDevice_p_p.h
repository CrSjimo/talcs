// SPDX-FileCopyrightText: 2026 CrSjimo
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TALCS_NATIVEAUDIODEVICE_P_P_H
#define TALCS_NATIVEAUDIODEVICE_P_P_H

#include <TalcsDevice/private/NativeAudioDevice_p.h>

#include <atomic>

#include <QMutex>
#include <QTimer>

#include <TalcsCore/AudioDataWrapper.h>
#include <TalcsCore/InterleavedAudioDataWrapper.h>
#include <TalcsDevice/private/AudioDevice_p.h>

namespace talcs {

    class TALCSDEVICE_EXPORT NativeAudioDevicePrivate : public AudioDevicePrivate {
        Q_DECLARE_PUBLIC(NativeAudioDevice)
    public:
        AudioDeviceCallback *callback = nullptr;
        QMutex callbackMutex;
        InterleavedAudioDataWrapper interleavedBuffer{nullptr, 0, 0};
        AudioDataWrapper planarBuffer{nullptr, 0, 0};
        std::atomic<int> streamError{0};
        QTimer *watchdog = nullptr;
    };

}

#endif
