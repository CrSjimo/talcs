// SPDX-FileCopyrightText: 2026 CrSjimo
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TALCS_ALSAAUDIODRIVER_H
#define TALCS_ALSAAUDIODRIVER_H

#include <TalcsDevice/AudioDriver.h>
#include <TalcsAlsa/TalcsAlsaGlobal.h>

namespace talcs {

    class AlsaAudioDriverPrivate;

    class TALCSALSA_EXPORT AlsaAudioDriver : public AudioDriver {
        Q_OBJECT
        Q_DECLARE_PRIVATE(AlsaAudioDriver)
    public:
        explicit AlsaAudioDriver(QObject *parent = nullptr);
        ~AlsaAudioDriver() override;
        bool initialize() override;
        void finalize() override;
        QStringList devices() const override;
        QString defaultDevice() const override;
        AudioDevice *createDevice(const QString &name) override;
    };

}

#endif
