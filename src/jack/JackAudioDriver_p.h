// SPDX-FileCopyrightText: 2026 CrSjimo
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TALCS_JACKAUDIODRIVER_P_H
#define TALCS_JACKAUDIODRIVER_P_H

#include <TalcsJack/JackAudioDriver.h>

#include <atomic>
#include <vector>

#include <QTimer>

#include <jack/jack.h>

#include <TalcsDevice/private/AudioDriver_p.h>
#include <TalcsDevice/private/NativeAudioDevice_p.h>

namespace talcs {

    struct JackEndpoint {
        QString name;
        QStringList ports;
    };

    class JackAudioDriverPrivate : public AudioDriverPrivate {
        Q_DECLARE_PUBLIC(JackAudioDriver)
    public:
        jack_client_t *client = nullptr;
        QList<JackEndpoint> endpoints;
        std::atomic<bool> dirty{true};
        std::atomic<bool> serverLost{false};
        std::atomic<jack_nframes_t> serverFrames{0};
        std::atomic<jack_nframes_t> serverRate{0};
        QTimer *timer = nullptr;
        jack_nframes_t frames = 0;
        jack_nframes_t rate = 0;
        void refresh();
    };

    class JackAudioDevice : public NativeAudioDevice {
        Q_OBJECT
    public:
        JackAudioDevice(const JackEndpoint &endpoint, JackAudioDriver *driver, jack_nframes_t frames, jack_nframes_t rate);
        ~JackAudioDevice() override;
        bool open(qint64 bufferSize, double sampleRate) override;
        void close() override;
        void updateEndpoint(const JackEndpoint &endpoint, jack_nframes_t frames, jack_nframes_t rate);

    protected:
        bool activateStream() override;
        void deactivateStream() override;

    private:
        JackEndpoint m_endpoint;
        jack_client_t *m_client = nullptr;
        std::vector<jack_port_t *> m_ports;
        std::vector<float *> m_output;
        bool m_active = false;
        std::atomic<bool> m_serverLost{false};
        int process(jack_nframes_t frames);
    };

}

#endif
