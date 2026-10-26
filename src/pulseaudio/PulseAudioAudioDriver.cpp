// SPDX-FileCopyrightText: 2026 CrSjimo
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "PulseAudioAudioDriver.h"
#include "PulseAudioAudioDriver_p.h"

#include <algorithm>
#include <cstdint>
#include <functional>

#include <pulse/timeval.h>

#include <TalcsDevice/private/NativeAudioDevice_p_p.h>

namespace talcs {

    // The caller holds the main-loop lock throughout the operation and timer lifetime.
    static bool waitFor(pa_threaded_mainloop *loop, const std::function<bool()> &pending) {
        struct Timeout {
            pa_threaded_mainloop *loop;
            bool expired = false;
        } timeout{loop};
        auto *api = pa_threaded_mainloop_get_api(loop);
        timeval deadline{};
        pa_gettimeofday(&deadline);
        pa_timeval_add(&deadline, 5000000);
        auto *timer = api->time_new(api, &deadline, [](pa_mainloop_api *, pa_time_event *, const timeval *, void *data) {
            auto *timeout = static_cast<Timeout *>(data);
            timeout->expired = true;
            pa_threaded_mainloop_signal(timeout->loop, 0);
        }, &timeout);
        if (!timer) {
            return false;
        }
        while (pending() && !timeout.expired) {
            pa_threaded_mainloop_wait(loop);
        }
        api->time_free(timer);
        return !timeout.expired;
    }

    static bool waitOperation(pa_threaded_mainloop *loop, pa_context *context, pa_operation *operation) {
        if (!operation) {
            return false;
        }
        const bool ready = waitFor(loop, [=] { return pa_operation_get_state(operation) == PA_OPERATION_RUNNING && PA_CONTEXT_IS_GOOD(pa_context_get_state(context)); });
        const bool completed = ready && pa_operation_get_state(operation) == PA_OPERATION_DONE && pa_context_get_state(context) == PA_CONTEXT_READY;
        if (!completed) {
            pa_operation_cancel(operation);
        }
        pa_operation_unref(operation);
        return completed;
    }

    PulseAudioAudioDriver::PulseAudioAudioDriver(QObject *parent) : AudioDriver(*new PulseAudioAudioDriverPrivate, parent) {
        Q_D(PulseAudioAudioDriver);
        setName(QStringLiteral("PulseAudio"));
        d->timer = new QTimer(this);
        d->timer->setInterval(250);
        connect(d->timer, &QTimer::timeout, this, [d] { d->refresh(); });
    }

    PulseAudioAudioDriver::~PulseAudioAudioDriver() {
        PulseAudioAudioDriver::finalize();
    }

    bool PulseAudioAudioDriver::initialize() {
        Q_D(PulseAudioAudioDriver);
        if (isInitialized()) {
            pa_threaded_mainloop_lock(d->loop);
            const bool connected = pa_context_get_state(d->context) == PA_CONTEXT_READY;
            pa_threaded_mainloop_unlock(d->loop);
            if (connected) {
                return true;
            }
        }
        finalize();
        d->loop = pa_threaded_mainloop_new();
        d->context = d->loop ? pa_context_new(pa_threaded_mainloop_get_api(d->loop), "TALCS") : nullptr;
        if (!d->context) {
            setErrorString(tr("The PulseAudio context could not be created."));
            finalize();
            return false;
        }
        pa_context_set_state_callback(d->context, [](pa_context *, void *data) { pa_threaded_mainloop_signal(static_cast<pa_threaded_mainloop *>(data), 0); }, d->loop);
        if (pa_threaded_mainloop_start(d->loop) < 0) {
            setErrorString(tr("The PulseAudio event loop could not be started."));
            finalize();
            return false;
        }
        d->loopStarted = true;
        pa_threaded_mainloop_lock(d->loop);
        bool ready = pa_context_connect(d->context, nullptr, PA_CONTEXT_NOAUTOSPAWN, nullptr) >= 0;
        ready = ready && waitFor(d->loop, [d] { const auto state = pa_context_get_state(d->context); return state != PA_CONTEXT_READY && PA_CONTEXT_IS_GOOD(state); });
        ready = ready && pa_context_get_state(d->context) == PA_CONTEXT_READY;
        if (ready) {
            pa_context_set_subscribe_callback(d->context, [](pa_context *, pa_subscription_event_type_t, uint32_t, void *data) {
                static_cast<PulseAudioAudioDriverPrivate *>(data)->dirty = true;
            }, d);
            d->subscriptionSucceeded = false;
            auto *operation = pa_context_subscribe(d->context, pa_subscription_mask_t(PA_SUBSCRIPTION_MASK_SINK | PA_SUBSCRIPTION_MASK_SERVER), [](pa_context *, int success, void *data) {
                auto *d = static_cast<PulseAudioAudioDriverPrivate *>(data);
                d->subscriptionSucceeded = success;
                pa_threaded_mainloop_signal(d->loop, 0);
            }, d);
            ready = waitOperation(d->loop, d->context, operation) && d->subscriptionSucceeded && d->enumerate();
        }
        const auto error = pa_context_errno(d->context);
        pa_threaded_mainloop_unlock(d->loop);
        if (!ready) {
            setErrorString(error ? QString::fromUtf8(pa_strerror(error)) : tr("PulseAudio device enumeration timed out."));
            finalize();
            return false;
        }
        AudioDriver::initialize();
        d->timer->start();
        setErrorString({});
        return true;
    }

    bool PulseAudioAudioDriverPrivate::enumerate() {
        pendingEndpoints.clear();
        enumerationSucceeded = false;
        auto *operation = pa_context_get_sink_info_list(context, [](pa_context *, const pa_sink_info *info, int end, void *data) {
            auto *d = static_cast<PulseAudioAudioDriverPrivate *>(data);
            if (end) {
                d->enumerationSucceeded = end > 0;
                pa_threaded_mainloop_signal(d->loop, 0);
            } else if (info && info->name && pa_sample_spec_valid(&info->sample_spec) && pa_channel_map_valid(&info->channel_map) && info->channel_map.channels == info->sample_spec.channels) {
                PulseAudioEndpoint endpoint;
                endpoint.id = QString::fromUtf8(info->name);
                const auto description = QString::fromUtf8(info->description);
                endpoint.name = description.isEmpty() ? endpoint.id : QStringLiteral("%1 [%2]").arg(description, endpoint.id);
                endpoint.channels = info->sample_spec.channels;
                endpoint.rate = info->sample_spec.rate;
                endpoint.channelMap = info->channel_map;
                d->pendingEndpoints.append(endpoint);
            }
        }, this);
        if (!waitOperation(loop, context, operation) || !enumerationSucceeded) {
            return false;
        }
        defaultId.clear();
        operation = pa_context_get_server_info(context, [](pa_context *, const pa_server_info *info, void *data) {
            auto *d = static_cast<PulseAudioAudioDriverPrivate *>(data);
            if (info) {
                d->defaultId = QString::fromUtf8(info->default_sink_name);
            }
            pa_threaded_mainloop_signal(d->loop, 0);
        }, this);
        if (!waitOperation(loop, context, operation)) {
            return false;
        }
        endpoints = pendingEndpoints;
        defaultName.clear();
        for (const auto &endpoint : endpoints) {
            if (endpoint.id == defaultId) {
                defaultName = endpoint.name;
                break;
            }
        }
        dirty = false;
        return true;
    }

    void PulseAudioAudioDriverPrivate::refresh() {
        Q_Q(PulseAudioAudioDriver);
        const auto previous = q->devices();
        const auto previousDefault = defaultName;
        pa_threaded_mainloop_lock(loop);
        const bool ready = pa_context_get_state(context) == PA_CONTEXT_READY;
        if (!ready || (dirty && !enumerate())) {
            endpoints.clear();
            defaultName.clear();
        }
        pa_threaded_mainloop_unlock(loop);
        for (auto *child : q->children()) {
            auto *device = qobject_cast<PulseAudioAudioDevice *>(child);
            if (device) {
                PulseAudioEndpoint current;
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

    void PulseAudioAudioDriver::finalize() {
        Q_D(PulseAudioAudioDriver);
        d->timer->stop();
        AudioDriver::finalize();
        for (auto *child : children()) {
            if (auto *device = qobject_cast<PulseAudioAudioDevice *>(child)) {
                device->close();
            }
        }
        if (d->loopStarted) {
            pa_threaded_mainloop_stop(d->loop);
            d->loopStarted = false;
        }
        if (d->context) {
            pa_context_set_state_callback(d->context, nullptr, nullptr);
            pa_context_set_subscribe_callback(d->context, nullptr, nullptr);
            pa_context_disconnect(d->context);
            pa_context_unref(d->context);
            d->context = nullptr;
        }
        if (d->loop) {
            pa_threaded_mainloop_free(d->loop);
            d->loop = nullptr;
        }
        d->endpoints.clear();
        d->defaultName.clear();
        d->dirty = true;
    }

    QStringList PulseAudioAudioDriver::devices() const {
        Q_D(const PulseAudioAudioDriver);
        QStringList names;
        for (const auto &endpoint : d->endpoints) {
            names.append(endpoint.name);
        }
        return names;
    }

    QString PulseAudioAudioDriver::defaultDevice() const {
        Q_D(const PulseAudioAudioDriver);
        return d->defaultName;
    }

    AudioDevice *PulseAudioAudioDriver::createDevice(const QString &name) {
        Q_D(PulseAudioAudioDriver);
        for (const auto &endpoint : d->endpoints) {
            if (endpoint.name == name) {
                return new PulseAudioAudioDevice(endpoint, this, d->loop, d->context);
            }
        }
        setErrorString(tr("The audio device is no longer available."));
        return nullptr;
    }

    PulseAudioAudioDevice::PulseAudioAudioDevice(const PulseAudioEndpoint &endpoint, PulseAudioAudioDriver *driver, pa_threaded_mainloop *loop, pa_context *context)
        : NativeAudioDevice(*new NativeAudioDevicePrivate, driver), m_endpoint(endpoint), m_loop(loop), m_context(context) {
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

    PulseAudioAudioDevice::~PulseAudioAudioDevice() {
        PulseAudioAudioDevice::close();
    }

    bool PulseAudioAudioDevice::open(qint64 bufferSize, double sampleRate) {
        close();
        if (!isInitialized() || !configure(bufferSize, sampleRate)) {
            return false;
        }
        pa_sample_spec format{PA_SAMPLE_FLOAT32NE, uint32_t(sampleRate), uint8_t(activeChannelCount())};
        if (!pa_sample_spec_valid(&format)) {
            setErrorString(tr("The requested audio format is invalid."));
            close();
            return false;
        }
        pa_threaded_mainloop_lock(m_loop);
        auto channelMap = m_endpoint.channelMap;
        channelMap.channels = format.channels;
        m_stream = pa_stream_new(m_context, "TALCS", &format, &channelMap);
        bool ready = m_stream != nullptr;
        if (ready) {
            pa_stream_set_state_callback(m_stream, [](pa_stream *stream, void *data) {
                auto *device = static_cast<PulseAudioAudioDevice *>(data);
                if (pa_stream_get_state(stream) == PA_STREAM_FAILED || pa_stream_get_state(stream) == PA_STREAM_TERMINATED) {
                    device->fail(pa_context_errno(device->m_context));
                }
                pa_threaded_mainloop_signal(device->m_loop, 0);
            }, this);
            pa_stream_set_write_callback(m_stream, [](pa_stream *, size_t bytes, void *data) { static_cast<PulseAudioAudioDevice *>(data)->write(bytes); }, this);
            const auto bytes = uint32_t(bufferSize * pa_frame_size(&format));
            const pa_buffer_attr attributes{uint32_t(-1), bytes * 2, 0, bytes, uint32_t(-1)};
            ready = pa_stream_connect_playback(m_stream, m_endpoint.id.toUtf8().constData(), &attributes, pa_stream_flags_t(PA_STREAM_START_CORKED | PA_STREAM_ADJUST_LATENCY | PA_STREAM_DONT_MOVE), nullptr, nullptr) >= 0;
            if (ready) {
                ready = waitFor(m_loop, [this] { return PA_CONTEXT_IS_GOOD(pa_context_get_state(m_context)) && PA_STREAM_IS_GOOD(pa_stream_get_state(m_stream)) && pa_stream_get_state(m_stream) != PA_STREAM_READY; });
                ready = ready && pa_stream_get_state(m_stream) == PA_STREAM_READY;
            }
        }
        const auto error = pa_context_errno(m_context);
        pa_threaded_mainloop_unlock(m_loop);
        if (!ready) {
            setErrorString(error ? QString::fromUtf8(pa_strerror(error)) : tr("The PulseAudio playback stream could not be connected."));
            close();
        }
        return ready;
    }

    bool PulseAudioAudioDevice::activateStream() {
        pa_threaded_mainloop_lock(m_loop);
        m_operationSuccess = false;
        auto *operation = pa_stream_cork(m_stream, 0, [](pa_stream *, int success, void *data) {
            auto *device = static_cast<PulseAudioAudioDevice *>(data);
            device->m_operationSuccess = success;
            pa_threaded_mainloop_signal(device->m_loop, 0);
        }, this);
        const bool ready = waitOperation(m_loop, m_context, operation) && m_operationSuccess;
        pa_threaded_mainloop_unlock(m_loop);
        if (!ready) {
            setErrorString(tr("The PulseAudio playback stream could not be started."));
        }
        return ready;
    }

    void PulseAudioAudioDevice::deactivateStream() {
        if (!m_stream) {
            return;
        }
        pa_threaded_mainloop_lock(m_loop);
        if (pa_stream_get_state(m_stream) == PA_STREAM_READY) {
            if (auto *operation = pa_stream_cork(m_stream, 1, nullptr, nullptr)) {
                pa_operation_unref(operation);
            }
            if (auto *operation = pa_stream_flush(m_stream, nullptr, nullptr)) {
                pa_operation_unref(operation);
            }
        }
        pa_threaded_mainloop_unlock(m_loop);
    }

    void PulseAudioAudioDevice::close() {
        stop();
        if (m_stream) {
            pa_threaded_mainloop_lock(m_loop);
            pa_stream_set_state_callback(m_stream, nullptr, nullptr);
            pa_stream_set_write_callback(m_stream, nullptr, nullptr);
            pa_stream_disconnect(m_stream);
            pa_stream_unref(m_stream);
            m_stream = nullptr;
            pa_threaded_mainloop_unlock(m_loop);
        }
        AudioDevice::close();
    }

    void PulseAudioAudioDevice::write(size_t bytes) {
        void *output = nullptr;
        if (pa_stream_begin_write(m_stream, &output, &bytes) < 0) {
            fail(pa_context_errno(m_context));
            return;
        }
        const auto frameSize = size_t(activeChannelCount()) * sizeof(float);
        bytes -= bytes % frameSize;
        if (!output || !bytes) {
            pa_stream_cancel_write(m_stream);
            return;
        }
        render(static_cast<float *>(output), qint64(bytes / frameSize));
        if (pa_stream_write(m_stream, output, bytes, nullptr, 0, PA_SEEK_RELATIVE) < 0) {
            fail(pa_context_errno(m_context));
        }
    }

    void PulseAudioAudioDevice::updateEndpoint(const PulseAudioEndpoint &endpoint) {
        const bool changed = endpoint.id != m_endpoint.id || endpoint.channels != channelCount() || !std::equal(m_endpoint.channelMap.map, m_endpoint.channelMap.map + m_endpoint.channels, endpoint.channelMap.map);
        const bool wasOpen = isOpen();
        if (changed) {
            close();
        }
        m_endpoint = endpoint;
        setIsInitialized(!endpoint.id.isEmpty() && endpoint.channels > 0);
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

    QString PulseAudioAudioDevice::streamErrorString(int error) const {
        return QString::fromUtf8(pa_strerror(error > 0 ? error : PA_ERR_CONNECTIONTERMINATED));
    }

}

extern "C" Q_DECL_EXPORT talcs::AudioDriver *talcsCreateAudioDriver(QObject *parent) {
    return new talcs::PulseAudioAudioDriver(parent);
}
