// SPDX-FileCopyrightText: 2026 CrSjimo
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TALCS_PULSEAUDIOAUDIODRIVER_H
#define TALCS_PULSEAUDIOAUDIODRIVER_H

#include <TalcsDevice/AudioDriver.h>
#include <TalcsPulseAudio/TalcsPulseAudioGlobal.h>

namespace talcs {

    class PulseAudioAudioDriverPrivate;

    class TALCSPULSEAUDIO_EXPORT PulseAudioAudioDriver : public AudioDriver {
        Q_OBJECT
        Q_DECLARE_PRIVATE(PulseAudioAudioDriver)
    public:
        explicit PulseAudioAudioDriver(QObject *parent = nullptr);
        ~PulseAudioAudioDriver() override;
        bool initialize() override;
        void finalize() override;
        QStringList devices() const override;
        QString defaultDevice() const override;
        AudioDevice *createDevice(const QString &name) override;
    };

}

#endif
