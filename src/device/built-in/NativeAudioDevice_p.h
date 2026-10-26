// SPDX-FileCopyrightText: 2026 CrSjimo
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TALCS_NATIVEAUDIODEVICE_P_H
#define TALCS_NATIVEAUDIODEVICE_P_H

#include <TalcsDevice/AudioDevice.h>

namespace talcs {

    class NativeAudioDevicePrivate;

    class TALCSDEVICE_EXPORT NativeAudioDevice : public AudioDevice {
        Q_OBJECT
        Q_DECLARE_PRIVATE(NativeAudioDevice)
    public:
        ~NativeAudioDevice() override;
        bool start(AudioDeviceCallback *callback) override;
        void stop() override;
        void lock() override;
        void unlock() override;

    protected:
        NativeAudioDevice(NativeAudioDevicePrivate &d, QObject *parent);
        bool configure(qint64 bufferSize, double sampleRate);
        void render(float *output, qint64 frames);
        void render(float *const *output, qint64 frames);
        void fail(int error);
        virtual bool activateStream() = 0;
        virtual void deactivateStream() = 0;
        virtual QString streamErrorString(int error) const;
    };

}

#endif
