// SPDX-FileCopyrightText: 2026 CrSjimo
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TALCS_PIPEWIREAUDIODRIVER_H
#define TALCS_PIPEWIREAUDIODRIVER_H

#include <TalcsDevice/AudioDriver.h>
#include <TalcsPipeWire/TalcsPipeWireGlobal.h>

namespace talcs {

    class PipeWireAudioDriverPrivate;

    class TALCSPIPEWIRE_EXPORT PipeWireAudioDriver : public AudioDriver {
        Q_OBJECT
        Q_DECLARE_PRIVATE(PipeWireAudioDriver)
    public:
        explicit PipeWireAudioDriver(QObject *parent = nullptr);
        ~PipeWireAudioDriver() override;
        bool initialize() override;
        void finalize() override;
        QStringList devices() const override;
        QString defaultDevice() const override;
        AudioDevice *createDevice(const QString &name) override;
    };

}

#endif
