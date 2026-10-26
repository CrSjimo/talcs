// SPDX-FileCopyrightText: 2026 CrSjimo
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "PipeWireAudioDriver.h"
#include "PipeWireAudioDriver_p.h"

#include <algorithm>
#include <cerrno>
#include <cstring>

#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <spa/param/audio/format-utils.h>

#include <TalcsDevice/private/NativeAudioDevice_p_p.h>

namespace talcs {

    static QString pipeWireProperty(const spa_dict *properties, const char *key) {
        return QString::fromUtf8(properties ? spa_dict_lookup(properties, key) : nullptr);
    }

    PipeWireAudioDriver::PipeWireAudioDriver(QObject *parent) : AudioDriver(*new PipeWireAudioDriverPrivate, parent) {
        Q_D(PipeWireAudioDriver);
        setName(QStringLiteral("PipeWire"));
        d->timer = new QTimer(this);
        d->timer->setInterval(250);
        connect(d->timer, &QTimer::timeout, this, [d] { d->refresh(); });
    }

    PipeWireAudioDriver::~PipeWireAudioDriver() {
        PipeWireAudioDriver::finalize();
    }

    bool PipeWireAudioDriverPrivate::synchronize() {
        const auto sequence = pw_core_sync(core, PW_ID_CORE, 0);
        if (sequence < 0) {
            return false;
        }
        timespec deadline{};
        pw_thread_loop_get_time(loop, &deadline, 5000000000LL);
        while (done != sequence && failure.isEmpty()) {
            if (pw_thread_loop_timed_wait_full(loop, &deadline) < 0) {
                return false;
            }
        }
        return failure.isEmpty();
    }

    bool PipeWireAudioDriver::initialize() {
        Q_D(PipeWireAudioDriver);
        if (isInitialized()) {
            pw_thread_loop_lock(d->loop);
            const bool connected = d->failure.isEmpty();
            pw_thread_loop_unlock(d->loop);
            if (connected) {
                return true;
            }
        }
        finalize();
        pw_init(nullptr, nullptr);
        d->libraryInitialized = true;
        d->failure.clear();
        d->done = -1;
        d->loop = pw_thread_loop_new("talcs-pipewire-registry", nullptr);
        if (!d->loop) {
            setErrorString(tr("The PipeWire event loop could not be created."));
            finalize();
            return false;
        }
        d->context = pw_context_new(pw_thread_loop_get_loop(d->loop), nullptr, 0);
        d->core = d->context ? pw_context_connect(d->context, nullptr, 0) : nullptr;
        if (!d->core) {
            setErrorString(tr("The PipeWire server could not be connected."));
            finalize();
            return false;
        }
        static const pw_core_events coreEvents = [] {
            pw_core_events events{};
            events.version = PW_VERSION_CORE_EVENTS;
            events.done = [](void *data, uint32_t id, int sequence) {
                auto *d = static_cast<PipeWireAudioDriverPrivate *>(data);
                if (id == PW_ID_CORE) {
                    d->done = sequence;
                    pw_thread_loop_signal(d->loop, false);
                }
            };
            events.error = [](void *data, uint32_t id, int, int result, const char *message) {
                auto *d = static_cast<PipeWireAudioDriverPrivate *>(data);
                if (id == PW_ID_CORE || result == -EPIPE) {
                    d->failure = QString::fromUtf8(message);
                    if (d->failure.isEmpty()) {
                        d->failure = QString::fromLocal8Bit(strerror(-result));
                    }
                    pw_thread_loop_signal(d->loop, false);
                }
            };
            return events;
        }();
        pw_core_add_listener(d->core, &d->coreListener, &coreEvents, d);
        d->registry = pw_core_get_registry(d->core, PW_VERSION_REGISTRY, 0);
        static const pw_registry_events registryEvents = [] {
            pw_registry_events events{};
            events.version = PW_VERSION_REGISTRY_EVENTS;
            events.global = [](void *data, uint32_t id, uint32_t, const char *type, uint32_t version, const spa_dict *properties) {
                auto *d = static_cast<PipeWireAudioDriverPrivate *>(data);
                if (!strcmp(type, PW_TYPE_INTERFACE_Metadata) && pipeWireProperty(properties, PW_KEY_METADATA_NAME) == QStringLiteral("default") && !d->metadata) {
                    d->metadata = static_cast<pw_metadata *>(pw_registry_bind(d->registry, id, type, std::min(version, uint32_t(PW_VERSION_METADATA)), 0));
                    if (!d->metadata) {
                        return;
                    }
                    d->metadataId = id;
                    static const pw_metadata_events metadataEvents = [] {
                        pw_metadata_events events{};
                        events.version = PW_VERSION_METADATA_EVENTS;
                        events.property = [](void *data, uint32_t subject, const char *key, const char *, const char *value) {
                            auto *d = static_cast<PipeWireAudioDriverPrivate *>(data);
                            if (subject == PW_ID_CORE && (!key || !strcmp(key, "default.audio.sink"))) {
                                d->defaultNode = QJsonDocument::fromJson(QByteArray(value ? value : "")).object().value(QStringLiteral("name")).toString();
                            }
                            return 0;
                        };
                        return events;
                    }();
                    pw_metadata_add_listener(d->metadata, &d->metadataListener, &metadataEvents, d);
                    return;
                }
                if (strcmp(type, PW_TYPE_INTERFACE_Node) || pipeWireProperty(properties, PW_KEY_MEDIA_CLASS) != QStringLiteral("Audio/Sink")) {
                    return;
                }
                auto endpoint = std::make_unique<PipeWireEndpoint>();
                endpoint->id = id;
                endpoint->nodeName = pipeWireProperty(properties, PW_KEY_NODE_NAME);
                const auto description = pipeWireProperty(properties, PW_KEY_NODE_DESCRIPTION);
                endpoint->name = description.isEmpty() ? endpoint->nodeName : QStringLiteral("%1 [%2]").arg(description, endpoint->nodeName);
                endpoint->channels = pipeWireProperty(properties, PW_KEY_AUDIO_CHANNELS).toInt();
                endpoint->node = static_cast<pw_node *>(pw_registry_bind(d->registry, id, type, std::min(version, uint32_t(PW_VERSION_NODE)), 0));
                if (!endpoint->node) {
                    return;
                }
                static const pw_node_events nodeEvents = [] {
                    pw_node_events events{};
                    events.version = PW_VERSION_NODE_EVENTS;
                    events.info = [](void *data, const pw_node_info *info) {
                        auto *endpoint = static_cast<PipeWireEndpoint *>(data);
                        const auto channels = pipeWireProperty(info->props, PW_KEY_AUDIO_CHANNELS).toInt();
                        if (channels > 0 && !endpoint->currentFormat) {
                            endpoint->channels = channels;
                        }
                    };
                    events.param = [](void *data, int, uint32_t id, uint32_t, uint32_t, const spa_pod *param) {
                        auto *endpoint = static_cast<PipeWireEndpoint *>(data);
                        if (id != SPA_PARAM_Format && endpoint->currentFormat) {
                            return;
                        }
                        spa_audio_info_raw format{};
                        if (param && spa_format_audio_raw_parse(param, &format) >= 0) {
                            if (format.channels > 0 && format.channels <= SPA_AUDIO_MAX_CHANNELS) {
                                endpoint->channels = int(format.channels);
                                endpoint->positions.clear();
                                if (!(format.flags & SPA_AUDIO_FLAG_UNPOSITIONED)) {
                                    for (uint32_t channel = 0; channel < format.channels; ++channel) {
                                        endpoint->positions.append(format.position[channel]);
                                    }
                                }
                            }
                            if (format.rate > 0) {
                                endpoint->rate = format.rate;
                            }
                            endpoint->currentFormat = id == SPA_PARAM_Format;
                        }
                    };
                    return events;
                }();
                pw_node_add_listener(endpoint->node, &endpoint->listener, &nodeEvents, endpoint.get());
                uint32_t params[] = {SPA_PARAM_EnumFormat, SPA_PARAM_Format};
                pw_node_subscribe_params(endpoint->node, params, 2);
                pw_node_enum_params(endpoint->node, 0, SPA_PARAM_EnumFormat, 0, UINT32_MAX, nullptr);
                d->nodes.emplace(id, std::move(endpoint));
            };
            events.global_remove = [](void *data, uint32_t id) {
                auto *d = static_cast<PipeWireAudioDriverPrivate *>(data);
                if (id == d->metadataId) {
                    spa_hook_remove(&d->metadataListener);
                    pw_proxy_destroy(reinterpret_cast<pw_proxy *>(d->metadata));
                    d->metadata = nullptr;
                    d->metadataId = SPA_ID_INVALID;
                    d->defaultNode.clear();
                }
                if (const auto it = d->nodes.find(id); it != d->nodes.end()) {
                    spa_hook_remove(&it->second->listener);
                    pw_proxy_destroy(reinterpret_cast<pw_proxy *>(it->second->node));
                    d->nodes.erase(it);
                }
            };
            return events;
        }();
        if (!d->registry) {
            setErrorString(tr("The PipeWire registry could not be created."));
            finalize();
            return false;
        }
        pw_registry_add_listener(d->registry, &d->registryListener, &registryEvents, d);
        if (pw_thread_loop_start(d->loop) < 0) {
            setErrorString(tr("The PipeWire event loop could not be started."));
            finalize();
            return false;
        }
        d->loopStarted = true;
        pw_thread_loop_lock(d->loop);
        const bool ready = d->synchronize() && d->synchronize();
        const auto failure = d->failure;
        pw_thread_loop_unlock(d->loop);
        if (!ready) {
            setErrorString(failure.isEmpty() ? tr("PipeWire device enumeration timed out.") : failure);
            finalize();
            return false;
        }
        AudioDriver::initialize();
        d->refresh();
        d->timer->start();
        setErrorString({});
        return true;
    }

    void PipeWireAudioDriverPrivate::refresh() {
        Q_Q(PipeWireAudioDriver);
        QList<PipeWireEndpoint> current;
        QString currentDefault;
        pw_thread_loop_lock(loop);
        if (failure.isEmpty()) {
            for (const auto &[id, endpoint] : nodes) {
                if (endpoint->channels > 0 && endpoint->channels <= SPA_AUDIO_MAX_CHANNELS && !endpoint->nodeName.isEmpty()) {
                    current.append({endpoint->id, endpoint->nodeName, endpoint->name, endpoint->channels, endpoint->rate, endpoint->positions});
                    if (endpoint->nodeName == defaultNode) {
                        currentDefault = endpoint->name;
                    }
                }
            }
        }
        pw_thread_loop_unlock(loop);
        const auto previous = q->devices();
        const auto previousDefault = defaultName;
        endpoints = current;
        defaultName = currentDefault;
        for (auto *child : q->children()) {
            auto *device = qobject_cast<PipeWireAudioDevice *>(child);
            if (device) {
                PipeWireEndpoint current;
                for (const auto &endpoint : endpoints) {
                    if (endpoint.name == device->name()) {
                        current = endpoint;
                        break;
                    }
                }
                device->updateEndpoint(current);
            }
        }
        if (q->devices() != previous || defaultName != previousDefault) {
            Q_EMIT q->deviceChanged();
        }
    }

    void PipeWireAudioDriver::finalize() {
        Q_D(PipeWireAudioDriver);
        d->timer->stop();
        AudioDriver::finalize();
        for (auto *child : children()) {
            if (auto *device = qobject_cast<PipeWireAudioDevice *>(child)) {
                device->close();
            }
        }
        if (d->loopStarted) {
            pw_thread_loop_stop(d->loop);
            d->loopStarted = false;
        }
        for (auto &[id, endpoint] : d->nodes) {
            spa_hook_remove(&endpoint->listener);
            pw_proxy_destroy(reinterpret_cast<pw_proxy *>(endpoint->node));
        }
        d->nodes.clear();
        if (d->metadata) {
            spa_hook_remove(&d->metadataListener);
            pw_proxy_destroy(reinterpret_cast<pw_proxy *>(d->metadata));
            d->metadata = nullptr;
        }
        d->metadataId = SPA_ID_INVALID;
        if (d->registry) {
            spa_hook_remove(&d->registryListener);
            pw_proxy_destroy(reinterpret_cast<pw_proxy *>(d->registry));
            d->registry = nullptr;
        }
        if (d->core) {
            spa_hook_remove(&d->coreListener);
            pw_core_disconnect(d->core);
            d->core = nullptr;
        }
        if (d->context) {
            pw_context_destroy(d->context);
            d->context = nullptr;
        }
        if (d->loop) {
            pw_thread_loop_destroy(d->loop);
            d->loop = nullptr;
        }
        d->endpoints.clear();
        d->defaultNode.clear();
        d->defaultName.clear();
        if (d->libraryInitialized) {
            pw_deinit();
            d->libraryInitialized = false;
        }
    }

    QStringList PipeWireAudioDriver::devices() const {
        Q_D(const PipeWireAudioDriver);
        QStringList names;
        for (const auto &endpoint : d->endpoints) {
            names.append(endpoint.name);
        }
        return names;
    }

    QString PipeWireAudioDriver::defaultDevice() const {
        Q_D(const PipeWireAudioDriver);
        return d->defaultName;
    }

    AudioDevice *PipeWireAudioDriver::createDevice(const QString &name) {
        Q_D(PipeWireAudioDriver);
        for (const auto &endpoint : d->endpoints) {
            if (endpoint.name == name) {
                return new PipeWireAudioDevice(endpoint, this);
            }
        }
        setErrorString(tr("The audio device is no longer available."));
        return nullptr;
    }

    PipeWireAudioDevice::PipeWireAudioDevice(const PipeWireEndpoint &endpoint, PipeWireAudioDriver *driver)
        : NativeAudioDevice(*new NativeAudioDevicePrivate, driver), m_endpoint(endpoint) {
        setDriver(driver);
        setName(endpoint.name);
        setChannelCount(endpoint.channels);
        setAvailableBufferSizes({64, 128, 256, 512, 1024, 2048, 4096, 8192});
        setPreferredBufferSize(1024);
        auto rates = QList<double>{8000, 11025, 16000, 22050, 32000, 44100, 48000, 88200, 96000, 176400, 192000};
        if (!rates.contains(endpoint.rate)) {
            rates.append(endpoint.rate);
            std::sort(rates.begin(), rates.end());
        }
        setAvailableSampleRates(rates);
        setPreferredSampleRate(endpoint.rate);
        setIsInitialized(true);
    }

    PipeWireAudioDevice::~PipeWireAudioDevice() {
        PipeWireAudioDevice::close();
    }

    bool PipeWireAudioDevice::open(qint64 bufferSize, double sampleRate) {
        close();
        if (!isInitialized() || !configure(bufferSize, sampleRate)) {
            return false;
        }
        m_loop = pw_thread_loop_new("talcs-pipewire-playback", nullptr);
        if (!m_loop) {
            setErrorString(tr("The PipeWire event loop could not be created."));
            close();
            return false;
        }
        static const pw_stream_events events = [] {
            pw_stream_events events{};
            events.version = PW_VERSION_STREAM_EVENTS;
            events.state_changed = [](void *data, pw_stream_state, pw_stream_state state, const char *) {
                auto *device = static_cast<PipeWireAudioDevice *>(data);
                if (device->m_connected && (state == PW_STREAM_STATE_ERROR || state == PW_STREAM_STATE_UNCONNECTED)) {
                    device->fail(-EPIPE);
                }
                pw_thread_loop_signal(device->m_loop, false);
            };
            events.param_changed = [](void *data, uint32_t id, const spa_pod *param) {
                auto *device = static_cast<PipeWireAudioDevice *>(data);
                if (id == SPA_PARAM_Format) {
                    spa_audio_info_raw format{};
                    device->m_formatValid = param && spa_format_audio_raw_parse(param, &format) >= 0 && format.format == SPA_AUDIO_FORMAT_F32 && format.rate == device->sampleRate() && format.channels == device->activeChannelCount();
                    if (param && !device->m_formatValid) {
                        device->fail(-EINVAL);
                    }
                }
            };
            events.process = [](void *data) { static_cast<PipeWireAudioDevice *>(data)->process(); };
            return events;
        }();
        auto *properties = pw_properties_new(PW_KEY_MEDIA_TYPE, "Audio", PW_KEY_MEDIA_CATEGORY, "Playback", PW_KEY_MEDIA_ROLE, "Music", PW_KEY_TARGET_OBJECT, m_endpoint.nodeName.toUtf8().constData(), PW_KEY_NODE_DONT_RECONNECT, "true", "node.dont-move", "true", nullptr);
        pw_properties_setf(properties, PW_KEY_NODE_LATENCY, "%lld/%d", static_cast<long long>(bufferSize), int(sampleRate));
        m_stream = pw_stream_new_simple(pw_thread_loop_get_loop(m_loop), "TALCS", properties, &events, this);
        if (!m_stream || pw_thread_loop_start(m_loop) < 0) {
            setErrorString(tr("The PipeWire playback stream could not be created."));
            close();
            return false;
        }
        m_loopStarted = true;
        pw_thread_loop_lock(m_loop);
        uint8_t storage[2048];
        spa_pod_builder builder = SPA_POD_BUILDER_INIT(storage, sizeof(storage));
        spa_audio_info_raw format{};
        format.format = SPA_AUDIO_FORMAT_F32;
        format.rate = uint32_t(sampleRate);
        format.channels = uint32_t(activeChannelCount());
        if (m_endpoint.positions.size() >= activeChannelCount()) {
            std::copy_n(m_endpoint.positions.cbegin(), activeChannelCount(), format.position);
        } else {
            format.flags = SPA_AUDIO_FLAG_UNPOSITIONED;
        }
        const spa_pod *params[] = {spa_format_audio_raw_build(&builder, SPA_PARAM_EnumFormat, &format)};
        bool ready = pw_stream_connect(m_stream, PW_DIRECTION_OUTPUT, PW_ID_ANY, pw_stream_flags(PW_STREAM_FLAG_AUTOCONNECT | PW_STREAM_FLAG_INACTIVE | PW_STREAM_FLAG_MAP_BUFFERS | PW_STREAM_FLAG_RT_PROCESS), params, 1) >= 0;
        timespec deadline{};
        pw_thread_loop_get_time(m_loop, &deadline, 5000000000LL);
        while (ready) {
            const auto state = pw_stream_get_state(m_stream, nullptr);
            if (state == PW_STREAM_STATE_PAUSED || state == PW_STREAM_STATE_STREAMING) {
                break;
            }
            if (state == PW_STREAM_STATE_ERROR || pw_thread_loop_timed_wait_full(m_loop, &deadline) < 0) {
                ready = false;
            }
        }
        m_connected = ready;
        pw_thread_loop_unlock(m_loop);
        if (!ready) {
            setErrorString(tr("The PipeWire playback stream could not be connected."));
            close();
        }
        return ready;
    }

    void PipeWireAudioDevice::close() {
        stop();
        if (m_loopStarted) {
            pw_thread_loop_stop(m_loop);
            m_loopStarted = false;
        }
        m_connected = false;
        if (m_stream) {
            pw_stream_destroy(m_stream);
            m_stream = nullptr;
        }
        if (m_loop) {
            pw_thread_loop_destroy(m_loop);
            m_loop = nullptr;
        }
        m_formatValid = false;
        AudioDevice::close();
    }

    bool PipeWireAudioDevice::activateStream() {
        if (!m_stream) {
            return false;
        }
        pw_thread_loop_lock(m_loop);
        const auto state = pw_stream_get_state(m_stream, nullptr);
        const bool ready = (state == PW_STREAM_STATE_PAUSED || state == PW_STREAM_STATE_STREAMING) && pw_stream_set_active(m_stream, true) >= 0;
        pw_thread_loop_unlock(m_loop);
        if (!ready) {
            setErrorString(tr("The PipeWire playback stream could not be started."));
        }
        return ready;
    }

    void PipeWireAudioDevice::deactivateStream() {
        if (m_stream) {
            pw_thread_loop_lock(m_loop);
            pw_stream_set_active(m_stream, false);
            pw_stream_flush(m_stream, false);
            pw_thread_loop_unlock(m_loop);
        }
    }

    void PipeWireAudioDevice::process() {
        auto *buffer = pw_stream_dequeue_buffer(m_stream);
        if (!buffer) {
            return;
        }
        auto *data = buffer->buffer;
        if (!data->n_datas || !data->datas[0].data || !data->datas[0].chunk) {
            pw_stream_queue_buffer(m_stream, buffer);
            fail(-EINVAL);
            return;
        }
        const auto stride = uint32_t(activeChannelCount() * sizeof(float));
        auto frames = data->datas[0].maxsize / stride;
        if (buffer->requested) {
            frames = uint32_t(std::min(uint64_t(frames), buffer->requested));
        }
        auto *output = static_cast<float *>(data->datas[0].data);
        if (m_formatValid) {
            render(output, frames);
        } else {
            std::fill_n(output, frames * activeChannelCount(), 0.0f);
        }
        data->datas[0].chunk->offset = 0;
        data->datas[0].chunk->stride = int32_t(stride);
        data->datas[0].chunk->size = frames * stride;
        buffer->size = frames;
        pw_stream_queue_buffer(m_stream, buffer);
    }

    void PipeWireAudioDevice::updateEndpoint(const PipeWireEndpoint &endpoint) {
        const bool changed = endpoint.nodeName != m_endpoint.nodeName || endpoint.id != m_endpoint.id || endpoint.channels != channelCount() || endpoint.positions != m_endpoint.positions;
        const bool wasOpen = isOpen();
        if (changed) {
            close();
        }
        m_endpoint = endpoint;
        setIsInitialized(!endpoint.nodeName.isEmpty() && endpoint.channels > 0);
        if (!isOpen()) {
            setChannelCount(endpoint.channels);
            setPreferredSampleRate(endpoint.rate);
            auto rates = availableSampleRates();
            if (endpoint.rate > 0 && !rates.contains(endpoint.rate)) {
                rates.append(endpoint.rate);
                std::sort(rates.begin(), rates.end());
                setAvailableSampleRates(rates);
            }
        }
        if (changed && wasOpen) {
            setErrorString(tr("The audio output device configuration changed."));
            Q_EMIT closed();
        }
    }

    QString PipeWireAudioDevice::streamErrorString(int error) const {
        return QString::fromLocal8Bit(strerror(-error));
    }

}

extern "C" Q_DECL_EXPORT talcs::AudioDriver *talcsCreateAudioDriver(QObject *parent) {
    return new talcs::PipeWireAudioDriver(parent);
}
