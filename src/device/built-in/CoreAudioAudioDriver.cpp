// SPDX-License-Identifier: LGPL-3.0-or-later

#include "CoreAudioAudioDriver_p.h"

#include <algorithm>
#include <utility>
#include <vector>

#include <QLoggingCategory>
#include <QMap>
#include <QMetaObject>
#include <QPointer>
#include <QSet>
#include <QTimer>

#include <TalcsDevice/private/CoreAudioAudioDevice_p.h>
#include <TalcsDevice/private/CoreAudioUtils_p.h>

namespace talcs {

    Q_LOGGING_CATEGORY(lcCoreAudioAudioDriver, "talcs.device.coreaudio.driver")

    CoreAudioAudioDriver::CoreAudioAudioDriver(ShareMode mode, QObject *parent)
        : AudioDriver(*new CoreAudioAudioDriverPrivate, parent) {
        Q_D(CoreAudioAudioDriver);
        d->mode = mode;
        const auto *driverName = mode == Shared
            ? QT_TRANSLATE_NOOP("talcs::AudioDriver", "CoreAudio")
            : QT_TRANSLATE_NOOP("talcs::AudioDriver", "CoreAudio (Hog Mode)");
        setName(QString::fromUtf8(driverName));
        d->refreshTimer = new QTimer(this);
        d->refreshTimer->setSingleShot(true);
        d->refreshTimer->setInterval(100);
        connect(d->refreshTimer, &QTimer::timeout, this, [this] { refreshDevices(); });
    }

    CoreAudioAudioDriver::~CoreAudioAudioDriver() {
        CoreAudioAudioDriver::finalize();
    }

    OSStatus CoreAudioAudioDriver::propertyChanged(AudioObjectID object, UInt32 count, const AudioObjectPropertyAddress addresses[], void *context) {
        auto *driver = static_cast<CoreAudioAudioDriver *>(context);
        bool failed = false;
        for (UInt32 i = 0; i < count; ++i) {
            failed |= addresses[i].mSelector == kAudioDevicePropertyIOStoppedAbnormally;
        }
        const auto eventTime = coreAudioEventTime();
        QMetaObject::invokeMethod(driver, [driver, object, failed, eventTime] {
            if (!driver->isInitialized()) {
                return;
            }
            if (failed) {
                driver->d_func()->failedDevices.insert(object, eventTime);
            }
            driver->requestRefresh();
        }, Qt::QueuedConnection);
        return noErr;
    }

    bool CoreAudioAudioDriver::initialize() {
        Q_D(CoreAudioAudioDriver);
        if (isInitialized()) {
            return true;
        }
        qCInfo(lcCoreAudioAudioDriver) << "CoreAudio driver initialization started." << "driver:" << name();
        const auto address = coreAudioPropertyAddress(kAudioHardwarePropertyDevices);
        const auto result = AudioObjectAddPropertyListener(kAudioObjectSystemObject, &address, propertyChanged, this);
        if (result != noErr) {
            setErrorString(coreAudioErrorString(result));
            qCWarning(lcCoreAudioAudioDriver) << "CoreAudio device notification registration failed." << "error:" << errorString();
            return false;
        }
        d->listeners.append({kAudioObjectSystemObject, address});
        AudioDriver::initialize();
        if (!refreshDevices(false)) {
            finalize();
            return false;
        }
        setErrorString({});
        qCInfo(lcCoreAudioAudioDriver) << "CoreAudio driver initialization completed." << "driver:" << name();
        return true;
    }

    void CoreAudioAudioDriver::finalize() {
        Q_D(CoreAudioAudioDriver);
        AudioDriver::finalize();
        d->refreshTimer->stop();
        for (const auto &listener : std::as_const(d->listeners)) {
            const auto result = AudioObjectRemovePropertyListener(listener.object, &listener.address, propertyChanged, this);
            if (result != noErr && result != kAudioHardwareBadObjectError && result != kAudioHardwareBadDeviceError) {
                qCWarning(lcCoreAudioAudioDriver) << "CoreAudio property notification unregistration failed." << "object:" << listener.object << "error:" << coreAudioErrorString(result);
            }
        }
        d->listeners.clear();
        const auto children = findChildren<CoreAudioAudioDevice *>(QString(), Qt::FindDirectChildrenOnly);
        for (auto *device : children) {
            device->close();
        }
        d->endpoints.clear();
        d->failedDevices.clear();
    }

    CoreAudioAudioDriver::ShareMode CoreAudioAudioDriver::shareMode() const {
        Q_D(const CoreAudioAudioDriver);
        return d->mode;
    }

    void CoreAudioAudioDriver::requestRefresh() {
        Q_D(CoreAudioAudioDriver);
        if (isInitialized() && !d->refreshTimer->isActive()) {
            d->refreshTimer->start();
        }
    }

    bool CoreAudioAudioDriver::refreshDevices(bool notifyChanges) {
        Q_D(CoreAudioAudioDriver);
        if (!isInitialized()) {
            return false;
        }
        std::vector<AudioDeviceID> ids;
        const auto result = coreAudioReadArray(kAudioObjectSystemObject, kAudioHardwarePropertyDevices, ids);
        if (result != noErr) {
            setErrorString(coreAudioErrorString(result));
            qCWarning(lcCoreAudioAudioDriver) << "CoreAudio device enumeration failed." << "driver:" << name() << "error:" << errorString();
            return false;
        }
        QList<CoreAudioEndpoint> endpoints;
        QMap<QString, int> nameCounts;
        for (const auto id : ids) {
            CoreAudioEndpoint endpoint;
            endpoint.id = id;
            UInt32 alive = 0;
            auto status = coreAudioReadProperty(id, kAudioDevicePropertyDeviceIsAlive, alive);
            if (status == noErr && alive) {
                status = coreAudioOutputChannelCount(id, endpoint.channels);
            }
            if (status != noErr || !alive || !endpoint.channels) {
                continue;
            }
            status = coreAudioReadString(id, kAudioDevicePropertyDeviceUID, endpoint.uid);
            if (status == noErr) {
                status = coreAudioReadProperty(id, kAudioDevicePropertyNominalSampleRate, endpoint.sampleRate);
            }
            if (status == noErr) {
                status = coreAudioReadProperty(id, kAudioDevicePropertyBufferFrameSize, endpoint.bufferSize);
            }
            if (status != noErr || endpoint.uid.isEmpty()) {
                qCWarning(lcCoreAudioAudioDriver) << "CoreAudio device property retrieval failed." << "device ID:" << id
                                                << "error:" << coreAudioErrorString(status != noErr ? status : kAudioHardwareBadPropertySizeError);
                continue;
            }
            if (coreAudioReadString(id, kAudioObjectPropertyName, endpoint.name) != noErr || endpoint.name.isEmpty()) {
                endpoint.name = endpoint.uid;
            }
            coreAudioReadProperty(id, kAudioDevicePropertyHogMode, endpoint.hogOwner);
            ++nameCounts[endpoint.name];
            endpoints.append(endpoint);
        }
        QMap<QString, QString> previousNames;
        QMap<QString, QString> currentNames;
        QSet<QString> usedNames;
        QSet<AudioObjectID> currentIds;
        for (const auto &endpoint : std::as_const(d->endpoints)) {
            previousNames.insert(endpoint.uid, endpoint.name);
        }
        for (auto &endpoint : endpoints) {
            if (nameCounts.value(endpoint.name) > 1 || usedNames.contains(endpoint.name)) {
                endpoint.name += QStringLiteral(" [%1]").arg(endpoint.uid);
            }
            while (usedNames.contains(endpoint.name)) {
                endpoint.name += QStringLiteral(" [%1]").arg(endpoint.uid);
            }
            usedNames.insert(endpoint.name);
            currentNames.insert(endpoint.uid, endpoint.name);
            currentIds.insert(endpoint.id);
        }
        for (auto it = d->listeners.begin(); it != d->listeners.end();) {
            if (it->object != kAudioObjectSystemObject && !currentIds.contains(it->object)) {
                AudioObjectRemovePropertyListener(it->object, &it->address, propertyChanged, this);
                it = d->listeners.erase(it);
            } else {
                ++it;
            }
        }
        const AudioObjectPropertyAddress addresses[] = {
            coreAudioPropertyAddress(kAudioObjectPropertyName),
            coreAudioPropertyAddress(kAudioDevicePropertyDeviceIsAlive),
            coreAudioPropertyAddress(kAudioDevicePropertyNominalSampleRate),
            coreAudioPropertyAddress(kAudioDevicePropertyBufferFrameSize),
            coreAudioPropertyAddress(kAudioDevicePropertyHogMode),
            coreAudioPropertyAddress(kAudioDevicePropertyIOStoppedAbnormally),
            coreAudioPropertyAddress(kAudioDevicePropertyStreamConfiguration, kAudioDevicePropertyScopeOutput),
            coreAudioPropertyAddress(kAudioDevicePropertyStreams, kAudioDevicePropertyScopeOutput),
        };
        for (const auto &endpoint : std::as_const(endpoints)) {
            for (const auto &address : addresses) {
                const bool registered = std::any_of(d->listeners.cbegin(), d->listeners.cend(), [&](const CoreAudioPropertyListener &listener) {
                    return listener.object == endpoint.id && listener.address.mSelector == address.mSelector && listener.address.mScope == address.mScope;
                });
                if (registered || !AudioObjectHasProperty(endpoint.id, &address)) {
                    continue;
                }
                const auto status = AudioObjectAddPropertyListener(endpoint.id, &address, propertyChanged, this);
                if (status == noErr) {
                    d->listeners.append({endpoint.id, address});
                } else {
                    qCWarning(lcCoreAudioAudioDriver) << "CoreAudio property notification registration failed." << "device:" << endpoint.name
                                                    << "property:" << address.mSelector << "error:" << coreAudioErrorString(status);
                }
            }
        }
        d->endpoints = endpoints;
        const auto failedDevices = std::exchange(d->failedDevices, {});
        QList<QPointer<CoreAudioAudioDevice>> devices;
        for (auto *device : findChildren<CoreAudioAudioDevice *>(QString(), Qt::FindDirectChildrenOnly)) {
            devices.append(device);
        }
        for (const auto &device : std::as_const(devices)) {
            if (device) {
                device->handleEndpointChange(endpoints, failedDevices);
            }
        }
        qCInfo(lcCoreAudioAudioDriver) << "CoreAudio device enumeration completed." << "driver:" << name() << "devices:" << currentNames.values();
        if (notifyChanges && currentNames != previousNames) {
            Q_EMIT deviceChanged();
        }
        return true;
    }

    QStringList CoreAudioAudioDriver::devices() const {
        Q_D(const CoreAudioAudioDriver);
        QStringList names;
        for (const auto &endpoint : d->endpoints) {
            names.append(endpoint.name);
        }
        return names;
    }

    QString CoreAudioAudioDriver::defaultDevice() const {
        Q_D(const CoreAudioAudioDriver);
        if (!isInitialized()) {
            return {};
        }
        AudioDeviceID id = kAudioObjectUnknown;
        const auto result = coreAudioReadProperty(kAudioObjectSystemObject, kAudioHardwarePropertyDefaultOutputDevice, id);
        if (result != noErr) {
            qCWarning(lcCoreAudioAudioDriver) << "CoreAudio default device retrieval failed." << "error:" << coreAudioErrorString(result);
            return {};
        }
        for (const auto &endpoint : d->endpoints) {
            if (endpoint.id == id) {
                return endpoint.name;
            }
        }
        return {};
    }

    AudioDevice *CoreAudioAudioDriver::createDevice(const QString &name) {
        Q_D(CoreAudioAudioDriver);
        if (!isInitialized() || name.isEmpty()) {
            return nullptr;
        }
        for (const auto &endpoint : d->endpoints) {
            if (endpoint.name == name) {
                return new CoreAudioAudioDevice(endpoint, this);
            }
        }
        qCWarning(lcCoreAudioAudioDriver) << "CoreAudio device creation failed because the device is unavailable." << "driver:" << this->name() << "device:" << name;
        return nullptr;
    }

}
