// SPDX-FileCopyrightText: 2026 CrSjimo
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TALCS_ALSAAUDIODRIVER_P_H
#define TALCS_ALSAAUDIODRIVER_P_H

#include <TalcsAlsa/AlsaAudioDriver.h>

#include <atomic>
#include <vector>

#include <QThread>
#include <QTimer>

#include <alsa/asoundlib.h>

#include <TalcsDevice/private/AudioDriver_p.h>
#include <TalcsDevice/private/NativeAudioDevice_p.h>

namespace talcs {

    struct AlsaEndpoint {
        QString id;
        QString name;
    };

    class AlsaAudioDriverPrivate : public AudioDriverPrivate {
        Q_DECLARE_PUBLIC(AlsaAudioDriver)
    public:
        QList<AlsaEndpoint> endpoints;
        QTimer *timer = nullptr;
        bool refresh();
    };

    class AlsaAudioDevice : public NativeAudioDevice {
        Q_OBJECT
    public:
        AlsaAudioDevice(const AlsaEndpoint &endpoint, AlsaAudioDriver *driver);
        ~AlsaAudioDevice() override;
        bool open(qint64 bufferSize, double sampleRate) override;
        void close() override;
        void updateEndpoint(const AlsaEndpoint &endpoint);

    protected:
        bool activateStream() override;
        void deactivateStream() override;
        QString streamErrorString(int error) const override;

    private:
        AlsaEndpoint m_endpoint;
        snd_pcm_t *m_pcm = nullptr;
        snd_pcm_format_t m_format = SND_PCM_FORMAT_UNKNOWN;
        QThread *m_worker = nullptr;
        std::atomic<bool> m_stopping{false};
        std::vector<float> m_output;
        std::vector<unsigned char> m_nativeOutput;
        int m_frameBytes = 0;
        bool probe();
        void process();
        void convert();
        bool recover(int error);
    };

}

#endif
