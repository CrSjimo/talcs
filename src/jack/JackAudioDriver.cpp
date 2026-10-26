// SPDX-FileCopyrightText: 2026 CrSjimo
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "JackAudioDriver.h"
#include "JackAudioDriver_p.h"

#include <algorithm>
#include <cerrno>
#include <limits>

#include <QCollator>
#include <QLocale>
#include <QMap>

#include <jack/metadata.h>

#include <TalcsDevice/private/NativeAudioDevice_p_p.h>

namespace talcs {

    JackAudioDriver::JackAudioDriver(QObject *parent) : AudioDriver(*new JackAudioDriverPrivate, parent) {
        Q_D(JackAudioDriver);
        setName(QStringLiteral("JACK"));
        d->timer = new QTimer(this);
        d->timer->setInterval(250);
        connect(d->timer, &QTimer::timeout, this, [d] { d->refresh(); });
    }

    JackAudioDriver::~JackAudioDriver() {
        JackAudioDriver::finalize();
    }

    bool JackAudioDriver::initialize() {
        Q_D(JackAudioDriver);
        if (isInitialized() && !d->serverLost.load()) {
            return true;
        }
        finalize();
        d->serverLost = false;
        d->dirty = true;
        jack_status_t status{};
        d->client = jack_client_open("TALCS Monitor", JackNoStartServer, &status);
        if (!d->client) {
            setErrorString(tr("The JACK server could not be connected (status %L1).").arg(int(status)));
            return false;
        }
        jack_on_shutdown(d->client, [](void *data) {
            auto *d = static_cast<JackAudioDriverPrivate *>(data);
            d->serverLost = true;
            d->dirty = true;
        }, d);
        d->serverFrames = jack_get_buffer_size(d->client);
        d->serverRate = jack_get_sample_rate(d->client);
        bool ready = jack_set_process_callback(d->client, [](jack_nframes_t, void *) { return 0; }, d) == 0;
        ready = ready && jack_set_port_registration_callback(d->client, [](jack_port_id_t, int, void *data) { static_cast<JackAudioDriverPrivate *>(data)->dirty = true; }, d) == 0;
        ready = ready && jack_set_port_rename_callback(d->client, [](jack_port_id_t, const char *, const char *, void *data) { static_cast<JackAudioDriverPrivate *>(data)->dirty = true; }, d) == 0;
        ready = ready && jack_set_buffer_size_callback(d->client, [](jack_nframes_t frames, void *data) {
            auto *d = static_cast<JackAudioDriverPrivate *>(data);
            d->serverFrames = frames;
            d->dirty = true;
            return 0;
        }, d) == 0;
        ready = ready && jack_set_sample_rate_callback(d->client, [](jack_nframes_t rate, void *data) {
            auto *d = static_cast<JackAudioDriverPrivate *>(data);
            d->serverRate = rate;
            d->dirty = true;
            return 0;
        }, d) == 0;
        if (!ready || jack_activate(d->client) != 0) {
            setErrorString(tr("The JACK notification client could not be activated."));
            finalize();
            return false;
        }
        AudioDriver::initialize();
        d->refresh();
        d->timer->start();
        setErrorString({});
        return true;
    }

    void JackAudioDriverPrivate::refresh() {
        Q_Q(JackAudioDriver);
        if (!dirty.exchange(false)) {
            return;
        }
        const auto previous = q->devices();
        const auto previousFrames = frames;
        const auto previousRate = rate;
        endpoints.clear();
        if (!serverLost.load()) {
            frames = serverFrames.load();
            rate = serverRate.load();
            const auto **ports = jack_get_ports(client, nullptr, JACK_DEFAULT_AUDIO_TYPE, JackPortIsPhysical | JackPortIsInput);
            struct Port {
                QString name;
                qlonglong order = std::numeric_limits<qlonglong>::max();
            };
            QMap<QString, QList<Port>> groups;
            if (ports) {
                for (int i = 0; ports[i]; ++i) {
                    const auto port = QString::fromUtf8(ports[i]);
                    const auto separator = port.indexOf(u':');
                    if (separator > 0) {
                        Port entry{port};
                        char *value = nullptr;
                        char *type = nullptr;
                        const auto *nativePort = jack_port_by_name(client, ports[i]);
                        if (nativePort && jack_get_property(jack_port_uuid(nativePort), JACK_METADATA_ORDER, &value, &type) == 0 && value) {
                            bool valid;
                            const auto order = QString::fromUtf8(value).toLongLong(&valid);
                            if (valid) {
                                entry.order = order;
                            }
                        }
                        jack_free(value);
                        jack_free(type);
                        groups[port.left(separator)].append(entry);
                    }
                }
                jack_free(ports);
            }
            QCollator collator{QLocale(QLocale::English)};
            collator.setNumericMode(true);
            for (auto it = groups.begin(); it != groups.end(); ++it) {
                std::sort(it->begin(), it->end(), [&collator](const Port &a, const Port &b) { return a.order != b.order ? a.order < b.order : collator.compare(a.name, b.name) < 0; });
                JackEndpoint endpoint{it.key(), {}};
                for (const auto &port : it.value()) {
                    endpoint.ports.append(port.name);
                }
                endpoints.append(endpoint);
            }
        }
        for (auto *child : q->children()) {
            auto *device = qobject_cast<JackAudioDevice *>(child);
            if (!device) {
                continue;
            }
            JackEndpoint current;
            for (const auto &endpoint : endpoints) {
                if (endpoint.name == device->name()) {
                    current = endpoint;
                    break;
                }
            }
            device->updateEndpoint(current, frames, rate);
        }
        if (previous != q->devices() || previousFrames != frames || previousRate != rate) {
            Q_EMIT q->deviceChanged();
        }
    }

    void JackAudioDriver::finalize() {
        Q_D(JackAudioDriver);
        d->timer->stop();
        AudioDriver::finalize();
        for (auto *child : children()) {
            if (auto *device = qobject_cast<JackAudioDevice *>(child)) {
                device->close();
            }
        }
        if (d->client) {
            jack_client_close(d->client);
            d->client = nullptr;
        }
        d->endpoints.clear();
    }

    QStringList JackAudioDriver::devices() const {
        Q_D(const JackAudioDriver);
        QStringList names;
        for (const auto &endpoint : d->endpoints) {
            names.append(endpoint.name);
        }
        return names;
    }

    QString JackAudioDriver::defaultDevice() const {
        const auto names = devices();
        return names.contains(QStringLiteral("system")) ? QStringLiteral("system") : names.value(0);
    }

    AudioDevice *JackAudioDriver::createDevice(const QString &name) {
        Q_D(JackAudioDriver);
        for (const auto &endpoint : d->endpoints) {
            if (endpoint.name == name) {
                return new JackAudioDevice(endpoint, this, d->frames, d->rate);
            }
        }
        setErrorString(tr("The audio device is no longer available."));
        return nullptr;
    }

    JackAudioDevice::JackAudioDevice(const JackEndpoint &endpoint, JackAudioDriver *driver, jack_nframes_t frames, jack_nframes_t rate)
        : NativeAudioDevice(*new NativeAudioDevicePrivate, driver), m_endpoint(endpoint) {
        setDriver(driver);
        setName(endpoint.name);
        setChannelCount(int(endpoint.ports.size()));
        setAvailableBufferSizes({frames});
        setPreferredBufferSize(frames);
        setAvailableSampleRates({double(rate)});
        setPreferredSampleRate(rate);
        setIsInitialized(frames > 0 && rate > 0 && !endpoint.ports.isEmpty());
    }

    JackAudioDevice::~JackAudioDevice() {
        JackAudioDevice::close();
    }

    bool JackAudioDevice::open(qint64 bufferSize, double sampleRate) {
        close();
        if (!isInitialized()) {
            setErrorString(tr("The audio device is no longer available."));
            return false;
        }
        jack_status_t status{};
        m_client = jack_client_open("TALCS", JackNoStartServer, &status);
        if (!m_client) {
            setErrorString(tr("The JACK server could not be connected (status %L1).").arg(int(status)));
            return false;
        }
        m_serverLost = false;
        jack_on_shutdown(m_client, [](void *data) {
            auto *device = static_cast<JackAudioDevice *>(data);
            device->m_serverLost = true;
            device->fail(-EPIPE);
        }, this);
        if (bufferSize != jack_get_buffer_size(m_client) || sampleRate != jack_get_sample_rate(m_client)) {
            setErrorString(tr("The requested format differs from the JACK server configuration."));
            close();
            return false;
        }
        if (!configure(bufferSize, sampleRate)) {
            close();
            return false;
        }
        bool ready = jack_set_process_callback(m_client, [](jack_nframes_t frames, void *data) { return static_cast<JackAudioDevice *>(data)->process(frames); }, this) == 0;
        ready = ready && jack_set_buffer_size_callback(m_client, [](jack_nframes_t frames, void *data) {
            auto *device = static_cast<JackAudioDevice *>(data);
            if (frames != device->bufferSize()) {
                device->fail(-EINVAL);
            }
            return 0;
        }, this) == 0;
        ready = ready && jack_set_sample_rate_callback(m_client, [](jack_nframes_t rate, void *data) {
            auto *device = static_cast<JackAudioDevice *>(data);
            if (rate != device->sampleRate()) {
                device->fail(-EINVAL);
            }
            return 0;
        }, this) == 0;
        for (int channel = 0; ready && channel < activeChannelCount(); ++channel) {
            auto *port = jack_port_register(m_client, qPrintable(QStringLiteral("out_%1").arg(channel + 1)), JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
            ready = port != nullptr;
            if (port) {
                m_ports.push_back(port);
            }
        }
        m_output.resize(m_ports.size());
        if (!ready) {
            setErrorString(tr("The JACK output ports could not be registered."));
            close();
        }
        return ready;
    }

    bool JackAudioDevice::activateStream() {
        if (m_serverLost.load() || jack_activate(m_client) != 0) {
            setErrorString(tr("The JACK playback client could not be activated."));
            return false;
        }
        m_active = true;
        for (size_t channel = 0; channel < m_ports.size(); ++channel) {
            const auto result = jack_connect(m_client, jack_port_name(m_ports[channel]), m_endpoint.ports.at(qsizetype(channel)).toUtf8().constData());
            if (result != 0 && result != EEXIST) {
                setErrorString(tr("The JACK output port could not be connected (error %L1).").arg(result));
                return false;
            }
        }
        return true;
    }

    void JackAudioDevice::deactivateStream() {
        if (m_client && m_active) {
            if (!m_serverLost.load()) {
                jack_deactivate(m_client);
            }
            m_active = false;
        }
    }

    void JackAudioDevice::close() {
        stop();
        if (m_client) {
            jack_client_close(m_client);
            m_client = nullptr;
        }
        m_ports.clear();
        m_output.clear();
        AudioDevice::close();
    }

    int JackAudioDevice::process(jack_nframes_t frames) {
        for (size_t channel = 0; channel < m_ports.size(); ++channel) {
            m_output[channel] = static_cast<float *>(jack_port_get_buffer(m_ports[channel], frames));
        }
        if (frames != bufferSize()) {
            for (auto *output : m_output) {
                std::fill_n(output, frames, 0.0f);
            }
            fail(-EINVAL);
            return 0;
        }
        render(m_output.data(), frames);
        return 0;
    }

    void JackAudioDevice::updateEndpoint(const JackEndpoint &endpoint, jack_nframes_t frames, jack_nframes_t rate) {
        if (endpoint.ports == m_endpoint.ports && frames == preferredBufferSize() && rate == preferredSampleRate()) {
            return;
        }
        const bool wasOpen = isOpen();
        close();
        m_endpoint = endpoint;
        setChannelCount(int(endpoint.ports.size()));
        setAvailableBufferSizes({frames});
        setPreferredBufferSize(frames);
        setAvailableSampleRates({double(rate)});
        setPreferredSampleRate(rate);
        setIsInitialized(!endpoint.ports.isEmpty() && frames > 0 && rate > 0);
        if (wasOpen) {
            setErrorString(tr("The JACK output ports or server configuration changed."));
            Q_EMIT closed();
        }
    }

}

extern "C" Q_DECL_EXPORT talcs::AudioDriver *talcsCreateAudioDriver(QObject *parent) {
    return new talcs::JackAudioDriver(parent);
}
