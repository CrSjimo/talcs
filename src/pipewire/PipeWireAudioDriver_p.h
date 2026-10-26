// SPDX-FileCopyrightText: 2026 CrSjimo
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TALCS_PIPEWIREAUDIODRIVER_P_H
#define TALCS_PIPEWIREAUDIODRIVER_P_H

#include <TalcsPipeWire/PipeWireAudioDriver.h>

#include <atomic>
#include <map>
#include <memory>

#include <QTimer>

#include <pipewire/extensions/metadata.h>
#include <pipewire/pipewire.h>

#include <TalcsDevice/private/AudioDriver_p.h>
#include <TalcsDevice/private/NativeAudioDevice_p.h>

namespace talcs {

    struct PipeWireEndpoint {
        uint32_t id = 0;
        QString nodeName;
        QString name;
        int channels = 0;
        double rate = 48000;
        QList<uint32_t> positions;
        bool currentFormat = false;
        pw_node *node = nullptr;
        spa_hook listener{};
    };

    class PipeWireAudioDriverPrivate : public AudioDriverPrivate {
        Q_DECLARE_PUBLIC(PipeWireAudioDriver)
    public:
        pw_thread_loop *loop = nullptr;
        pw_context *context = nullptr;
        pw_core *core = nullptr;
        pw_registry *registry = nullptr;
        pw_metadata *metadata = nullptr;
        uint32_t metadataId = SPA_ID_INVALID;
        spa_hook coreListener{};
        spa_hook registryListener{};
        spa_hook metadataListener{};
        std::map<uint32_t, std::unique_ptr<PipeWireEndpoint>> nodes;
        QList<PipeWireEndpoint> endpoints;
        QString defaultNode;
        QString defaultName;
        QString failure;
        bool libraryInitialized = false;
        bool loopStarted = false;
        int done = -1;
        QTimer *timer = nullptr;

        bool synchronize();
        void refresh();
    };

    class PipeWireAudioDevice : public NativeAudioDevice {
        Q_OBJECT
    public:
        PipeWireAudioDevice(const PipeWireEndpoint &endpoint, PipeWireAudioDriver *driver);
        ~PipeWireAudioDevice() override;
        bool open(qint64 bufferSize, double sampleRate) override;
        void close() override;
        void updateEndpoint(const PipeWireEndpoint &endpoint);

    protected:
        bool activateStream() override;
        void deactivateStream() override;
        QString streamErrorString(int error) const override;

    private:
        PipeWireEndpoint m_endpoint;
        pw_thread_loop *m_loop = nullptr;
        pw_stream *m_stream = nullptr;
        bool m_connected = false;
        bool m_loopStarted = false;
        std::atomic<bool> m_formatValid{false};
        void process();
    };

}

#endif
