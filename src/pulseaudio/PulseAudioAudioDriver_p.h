// SPDX-FileCopyrightText: 2026 CrSjimo
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TALCS_PULSEAUDIOAUDIODRIVER_P_H
#define TALCS_PULSEAUDIOAUDIODRIVER_P_H

#include <TalcsPulseAudio/PulseAudioAudioDriver.h>

#include <QTimer>

#include <pulse/pulseaudio.h>

#include <TalcsDevice/private/AudioDriver_p.h>
#include <TalcsDevice/private/NativeAudioDevice_p.h>

namespace talcs {

    struct PulseAudioEndpoint {
        QString id;
        QString name;
        int channels = 0;
        double rate = 0;
        pa_channel_map channelMap{};
    };

    class PulseAudioAudioDriverPrivate : public AudioDriverPrivate {
        Q_DECLARE_PUBLIC(PulseAudioAudioDriver)
    public:
        pa_threaded_mainloop *loop = nullptr;
        pa_context *context = nullptr;
        QList<PulseAudioEndpoint> endpoints;
        QList<PulseAudioEndpoint> pendingEndpoints;
        QString defaultId;
        QString defaultName;
        bool dirty = true;
        bool enumerationSucceeded = false;
        bool subscriptionSucceeded = false;
        bool loopStarted = false;
        QTimer *timer = nullptr;
        bool enumerate();
        void refresh();
    };

    class PulseAudioAudioDevice : public NativeAudioDevice {
        Q_OBJECT
    public:
        PulseAudioAudioDevice(const PulseAudioEndpoint &endpoint, PulseAudioAudioDriver *driver, pa_threaded_mainloop *loop, pa_context *context);
        ~PulseAudioAudioDevice() override;
        bool open(qint64 bufferSize, double sampleRate) override;
        void close() override;
        void updateEndpoint(const PulseAudioEndpoint &endpoint);

    protected:
        bool activateStream() override;
        void deactivateStream() override;
        QString streamErrorString(int error) const override;

    private:
        PulseAudioEndpoint m_endpoint;
        pa_threaded_mainloop *m_loop = nullptr;
        pa_context *m_context = nullptr;
        pa_stream *m_stream = nullptr;
        bool m_operationSuccess = false;
        void write(size_t bytes);
    };

}

#endif
