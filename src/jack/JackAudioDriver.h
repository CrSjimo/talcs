// SPDX-FileCopyrightText: 2026 CrSjimo
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TALCS_JACKAUDIODRIVER_H
#define TALCS_JACKAUDIODRIVER_H

#include <TalcsDevice/AudioDriver.h>
#include <TalcsJack/TalcsJackGlobal.h>

namespace talcs {

    class JackAudioDriverPrivate;

    class TALCSJACK_EXPORT JackAudioDriver : public AudioDriver {
        Q_OBJECT
        Q_DECLARE_PRIVATE(JackAudioDriver)
    public:
        explicit JackAudioDriver(QObject *parent = nullptr);
        ~JackAudioDriver() override;
        bool initialize() override;
        void finalize() override;
        QStringList devices() const override;
        QString defaultDevice() const override;
        AudioDevice *createDevice(const QString &name) override;
    };

}

#endif
