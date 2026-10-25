// SPDX-License-Identifier: LGPL-3.0-or-later

// MMDevice property key definitions require INITGUID before the first inclusion of mmdeviceapi.h.
#define INITGUID
#include "WASAPIAudioDriver_p.h"
#undef INITGUID

#include <functiondiscoverykeys_devpkey.h>
#include <propvarutil.h>

#include <QDebug>
#include <QLoggingCategory>
#include <QMap>
#include <QPointer>
#include <QSet>
#include <QTimer>

#include <TalcsDevice/private/WASAPIAudioDevice_p.h>
#include <TalcsDevice/private/WASAPIAudioUtils_p.h>

namespace talcs {

    Q_LOGGING_CATEGORY(lcWASAPIAudioDriver, "talcs.device.wasapi.driver")

    using Microsoft::WRL::ComPtr;

    HRESULT STDMETHODCALLTYPE WASAPIEndpointNotification::QueryInterface(REFIID iid, void **object) {
        if (!object) {
            return E_POINTER;
        }
        *object = nullptr;
        if (iid == __uuidof(IUnknown) || iid == __uuidof(IMMNotificationClient)) {
            *object = static_cast<IMMNotificationClient *>(this);
            AddRef();
            return S_OK;
        }
        return E_NOINTERFACE;
    }

    ULONG STDMETHODCALLTYPE WASAPIEndpointNotification::AddRef() {
        return ++m_referenceCount;
    }

    ULONG STDMETHODCALLTYPE WASAPIEndpointNotification::Release() {
        const auto count = --m_referenceCount;
        if (!count) {
            delete this;
        }
        return count;
    }

    HRESULT STDMETHODCALLTYPE WASAPIEndpointNotification::OnDeviceStateChanged(LPCWSTR id, DWORD state) {
        qCDebug(lcWASAPIAudioDriver) << "WASAPI endpoint state changed." << "endpoint:" << QString::fromWCharArray(id) << "state:" << state;
        Q_EMIT endpointChanged();
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE WASAPIEndpointNotification::OnDeviceAdded(LPCWSTR id) {
        qCDebug(lcWASAPIAudioDriver) << "WASAPI endpoint added." << "endpoint:" << QString::fromWCharArray(id);
        Q_EMIT endpointChanged();
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE WASAPIEndpointNotification::OnDeviceRemoved(LPCWSTR id) {
        qCDebug(lcWASAPIAudioDriver) << "WASAPI endpoint removed." << "endpoint:" << QString::fromWCharArray(id);
        Q_EMIT endpointChanged();
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE WASAPIEndpointNotification::OnDefaultDeviceChanged(EDataFlow, ERole, LPCWSTR) {
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE WASAPIEndpointNotification::OnPropertyValueChanged(LPCWSTR id, const PROPERTYKEY key) {
        const bool formatChanged = IsEqualPropertyKey(key, PKEY_AudioEngine_DeviceFormat) || IsEqualPropertyKey(key, PKEY_AudioEngine_OEMFormat);
        if (formatChanged || IsEqualPropertyKey(key, PKEY_Device_FriendlyName)) {
            qCDebug(lcWASAPIAudioDriver) << "WASAPI endpoint properties changed." << "endpoint:" << QString::fromWCharArray(id) << "format changed:" << formatChanged;
            Q_EMIT endpointChanged();
        }
        return S_OK;
    }

    WASAPIAudioDriver::WASAPIAudioDriver(ShareMode mode, QObject *parent)
        : AudioDriver(*new WASAPIAudioDriverPrivate, parent) {
        Q_D(WASAPIAudioDriver);
        d->mode = mode;
        const auto *driverName = mode == Shared
            ? QT_TRANSLATE_NOOP("talcs::AudioDriver", "WASAPI")
            : QT_TRANSLATE_NOOP("talcs::AudioDriver", "WASAPI (Exclusive Mode)");
        setName(QString::fromUtf8(driverName));
        d->refreshTimer = new QTimer(this);
        d->refreshTimer->setSingleShot(true);
        d->refreshTimer->setInterval(100);
        connect(d->refreshTimer, &QTimer::timeout, this, [this] { refreshDevices(); });
    }

    WASAPIAudioDriver::~WASAPIAudioDriver() {
        WASAPIAudioDriver::finalize();
    }

    bool WASAPIAudioDriver::initialize() {
        Q_D(WASAPIAudioDriver);
        if (isInitialized()) {
            qCDebug(lcWASAPIAudioDriver) << "WASAPI driver is already initialized." << "driver:" << name();
            return true;
        }
        qCInfo(lcWASAPIAudioDriver) << "WASAPI driver initialization started." << "driver:" << name();
        auto result = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        d->ownsCom = SUCCEEDED(result);
        if (FAILED(result) && result != RPC_E_CHANGED_MODE) {
            setErrorString(wasapiErrorString(result));
            qCWarning(lcWASAPIAudioDriver) << "WASAPI COM initialization failed." << "driver:" << name() << "error:" << errorString();
            return false;
        }
        result = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&d->enumerator));
        if (FAILED(result)) {
            qCWarning(lcWASAPIAudioDriver) << "WASAPI endpoint enumerator creation failed." << "driver:" << name() << "error:" << wasapiErrorString(result);
        }
        if (SUCCEEDED(result)) {
            d->notification = new WASAPIEndpointNotification;
            connect(d->notification, &WASAPIEndpointNotification::endpointChanged, this, &WASAPIAudioDriver::requestRefresh, Qt::QueuedConnection);
            result = d->enumerator->RegisterEndpointNotificationCallback(d->notification);
            if (FAILED(result)) {
                qCWarning(lcWASAPIAudioDriver) << "WASAPI endpoint notification registration failed." << "driver:" << name() << "error:" << wasapiErrorString(result);
            }
        }
        if (FAILED(result)) {
            setErrorString(wasapiErrorString(result));
            finalize();
            return false;
        }
        AudioDriver::initialize();
        if (!refreshDevices(false)) {
            finalize();
            return false;
        }
        setErrorString({});
        qCInfo(lcWASAPIAudioDriver) << "WASAPI driver initialization completed." << "driver:" << name();
        return true;
    }

    void WASAPIAudioDriver::finalize() {
        Q_D(WASAPIAudioDriver);
        if (isInitialized() || d->enumerator) {
            qCInfo(lcWASAPIAudioDriver) << "WASAPI driver finalization started." << "driver:" << name();
        }
        d->refreshTimer->stop();
        AudioDriver::finalize();
        if (d->notification) {
            disconnect(d->notification, nullptr, this, nullptr);
            if (d->enumerator) {
                const auto result = d->enumerator->UnregisterEndpointNotificationCallback(d->notification);
                if (FAILED(result)) {
                    qCWarning(lcWASAPIAudioDriver) << "WASAPI endpoint notification unregistration failed." << "driver:" << name() << "error:" << wasapiErrorString(result);
                }
            }
            d->notification->Release();
            d->notification = nullptr;
        }
        const auto children = findChildren<WASAPIAudioDevice *>(QString(), Qt::FindDirectChildrenOnly);
        for (auto *device : children) {
            device->close();
        }
        d->endpoints.clear();
        d->enumerator.Reset();
        if (d->ownsCom) {
            CoUninitialize();
            d->ownsCom = false;
        }
    }

    WASAPIAudioDriver::ShareMode WASAPIAudioDriver::shareMode() const {
        Q_D(const WASAPIAudioDriver);
        return d->mode;
    }

    void WASAPIAudioDriver::requestRefresh() {
        Q_D(WASAPIAudioDriver);
        if (isInitialized()) {
            d->refreshTimer->start();
        }
    }

    bool WASAPIAudioDriver::refreshDevices(bool notifyChanges) {
        Q_D(WASAPIAudioDriver);
        if (!isInitialized()) {
            qCWarning(lcWASAPIAudioDriver) << "WASAPI device enumeration requires an initialized driver." << "driver:" << name();
            return false;
        }
        qCInfo(lcWASAPIAudioDriver) << "WASAPI device enumeration started." << "driver:" << name();
        ComPtr<IMMDeviceCollection> collection;
        auto result = d->enumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &collection);
        UINT count = 0;
        if (SUCCEEDED(result)) {
            result = collection->GetCount(&count);
        }
        if (FAILED(result)) {
            setErrorString(wasapiErrorString(result));
            qCWarning(lcWASAPIAudioDriver) << "WASAPI endpoint enumeration failed." << "driver:" << name() << "error:" << errorString();
            return false;
        }
        QMap<QString, WASAPIEndpoint> previousEndpoints;
        QMap<QString, QString> previousNames;
        for (const auto &endpoint : d->endpoints) {
            previousEndpoints.insert(endpoint.id, endpoint);
            previousNames.insert(endpoint.id, endpoint.name);
        }
        QList<WASAPIEndpoint> endpoints;
        QMap<QString, int> nameCounts;
        for (UINT i = 0; i < count; ++i) {
            ComPtr<IMMDevice> endpoint;
            result = collection->Item(i, &endpoint);
            if (FAILED(result)) {
                qCWarning(lcWASAPIAudioDriver) << "WASAPI endpoint retrieval failed." << "driver:" << name() << "index:" << i << "error:" << wasapiErrorString(result);
                continue;
            }
            const auto id = wasapiEndpointId(endpoint.Get(), &result);
            if (id.isEmpty()) {
                qCWarning(lcWASAPIAudioDriver) << "WASAPI endpoint ID retrieval failed." << "driver:" << name() << "index:" << i << "error:" << wasapiErrorString(result);
                continue;
            }
            const auto previousEndpoint = previousEndpoints.value(id);
            auto name = previousEndpoint.friendlyName;
            auto format = previousEndpoint.format;
            ComPtr<IPropertyStore> properties;
            result = endpoint->OpenPropertyStore(STGM_READ, &properties);
            if (SUCCEEDED(result)) {
                PROPVARIANT value;
                PropVariantInit(&value);
                result = properties->GetValue(PKEY_Device_FriendlyName, &value);
                if (SUCCEEDED(result) && value.vt == VT_LPWSTR) {
                    name = QString::fromWCharArray(value.pwszVal);
                } else {
                    qCWarning(lcWASAPIAudioDriver) << "WASAPI endpoint name retrieval failed." << "endpoint:" << id
                                                 << "error:" << wasapiErrorString(FAILED(result) ? result : E_UNEXPECTED);
                }
                PropVariantClear(&value);
                PropVariantInit(&value);
                result = properties->GetValue(PKEY_AudioEngine_DeviceFormat, &value);
                if (SUCCEEDED(result) && value.vt == VT_BLOB && value.blob.pBlobData) {
                    format = QByteArray(reinterpret_cast<const char *>(value.blob.pBlobData), static_cast<int>(value.blob.cbSize));
                } else {
                    qCWarning(lcWASAPIAudioDriver) << "WASAPI endpoint format retrieval failed." << "device:" << name << "endpoint:" << id
                                                 << "error:" << wasapiErrorString(FAILED(result) ? result : E_UNEXPECTED);
                }
                PropVariantClear(&value);
            } else {
                qCWarning(lcWASAPIAudioDriver) << "WASAPI endpoint property store open failed." << "endpoint:" << id << "error:" << wasapiErrorString(result);
            }
            if (name.isEmpty()) {
                name = id;
            }
            ++nameCounts[name];
            endpoints.append({id, name, name, format});
        }
        QSet<QString> usedNames;
        for (auto &endpoint : endpoints) {
            // Endpoint IDs distinguish devices with identical friendly names.
            if (nameCounts.value(endpoint.name) > 1 || usedNames.contains(endpoint.name)) {
                endpoint.name += QStringLiteral(" [%1]").arg(endpoint.id);
            }
            while (usedNames.contains(endpoint.name)) {
                endpoint.name += QStringLiteral(" [%1]").arg(endpoint.id);
            }
            usedNames.insert(endpoint.name);
            qCInfo(lcWASAPIAudioDriver) << "WASAPI device enumerated." << "driver:" << name() << "device:" << endpoint.name << "endpoint:" << endpoint.id;
        }
        QMap<QString, QString> currentNames;
        QSet<QString> changedFormats;
        for (const auto &endpoint : endpoints) {
            currentNames.insert(endpoint.id, endpoint.name);
            if (previousEndpoints.contains(endpoint.id) && previousEndpoints.value(endpoint.id).format != endpoint.format) {
                changedFormats.insert(endpoint.id);
            }
        }
        d->endpoints = endpoints;
        qCInfo(lcWASAPIAudioDriver) << "WASAPI device enumeration completed." << "driver:" << name() << "device count:" << endpoints.size();
        QList<QPointer<WASAPIAudioDevice>> devices;
        for (auto *device : findChildren<WASAPIAudioDevice *>(QString(), Qt::FindDirectChildrenOnly)) {
            devices.append(device);
        }
        for (const auto &device : devices) {
            if (device) {
                device->handleEndpointChange(d->endpoints, changedFormats);
            }
        }
        if (notifyChanges && currentNames != previousNames) {
            qCInfo(lcWASAPIAudioDriver) << "WASAPI device list changed." << "driver:" << name() << "previous devices:" << previousNames.values() << "devices:" << currentNames.values();
            Q_EMIT deviceChanged();
        }
        return true;
    }

    QStringList WASAPIAudioDriver::devices() const {
        Q_D(const WASAPIAudioDriver);
        QStringList names;
        for (const auto &endpoint : d->endpoints) {
            names.append(endpoint.name);
        }
        return names;
    }

    QString WASAPIAudioDriver::defaultDevice() const {
        Q_D(const WASAPIAudioDriver);
        if (!isInitialized()) {
            return {};
        }
        ComPtr<IMMDevice> defaultEndpoint;
        auto result = d->enumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &defaultEndpoint);
        if (FAILED(result)) {
            qCWarning(lcWASAPIAudioDriver) << "WASAPI default device retrieval failed." << "driver:" << name() << "error:" << wasapiErrorString(result);
            return {};
        }
        const auto defaultId = wasapiEndpointId(defaultEndpoint.Get(), &result);
        if (FAILED(result)) {
            qCWarning(lcWASAPIAudioDriver) << "WASAPI default endpoint ID retrieval failed." << "driver:" << name() << "error:" << wasapiErrorString(result);
        }
        for (const auto &endpoint : d->endpoints) {
            if (endpoint.id == defaultId) {
                qCDebug(lcWASAPIAudioDriver) << "WASAPI default device resolved." << "driver:" << name() << "device:" << endpoint.name;
                return endpoint.name;
            }
        }
        return {};
    }

    AudioDevice *WASAPIAudioDriver::createDevice(const QString &name) {
        Q_D(WASAPIAudioDriver);
        qCInfo(lcWASAPIAudioDriver) << "WASAPI device creation started." << "driver:" << this->name() << "device:" << name;
        if (!isInitialized() || name.isEmpty()) {
            qCWarning(lcWASAPIAudioDriver) << "WASAPI device creation requires an initialized driver and a nonempty device name."
                                         << "driver:" << this->name() << "device:" << name << "initialized:" << isInitialized();
            return nullptr;
        }
        for (const auto &endpoint : d->endpoints) {
            if (endpoint.name == name) {
                return new WASAPIAudioDevice(name, endpoint.id, this);
            }
        }
        qCWarning(lcWASAPIAudioDriver) << "WASAPI device creation failed because the device is unavailable." << "driver:" << this->name() << "device:" << name;
        return nullptr;
    }

}
