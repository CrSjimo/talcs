// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TALCS_WASAPIAUDIODRIVER_P_H
#define TALCS_WASAPIAUDIODRIVER_P_H

#include <atomic>

#include <QByteArray>
#include <QList>
#include <qt_windows.h>

#include <mmdeviceapi.h>
#include <wrl/client.h>

#include <TalcsDevice/private/AudioDriver_p.h>

QT_BEGIN_NAMESPACE
class QTimer;
QT_END_NAMESPACE

namespace talcs {

    class WASAPIAudioDriverPrivate;

    class WASAPIAudioDriver : public AudioDriver {
        Q_OBJECT
        Q_DECLARE_PRIVATE(WASAPIAudioDriver)
    public:
        enum ShareMode {
            Shared,
            Exclusive,
        };

        explicit WASAPIAudioDriver(ShareMode mode, QObject *parent = nullptr);
        ~WASAPIAudioDriver() override;

        bool initialize() override;
        void finalize() override;
        QStringList devices() const override;
        QString defaultDevice() const override;
        AudioDevice *createDevice(const QString &name) override;

        ShareMode shareMode() const;
        void requestRefresh();

    private:
        bool refreshDevices(bool notifyChanges = true);
    };

    class WASAPIEndpointNotification : public QObject, public IMMNotificationClient {
        Q_OBJECT
    public:
        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void **object) override;
        ULONG STDMETHODCALLTYPE AddRef() override;
        ULONG STDMETHODCALLTYPE Release() override;
        HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(LPCWSTR id, DWORD state) override;
        HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR id) override;
        HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR id) override;
        HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR id) override;
        HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(LPCWSTR id, const PROPERTYKEY key) override;

    Q_SIGNALS:
        void endpointChanged();

    private:
        std::atomic<ULONG> m_referenceCount{1};
    };

    struct WASAPIEndpoint {
        QString id;
        QString name;
        QString friendlyName;
        QByteArray format;
    };

    class WASAPIAudioDriverPrivate : public AudioDriverPrivate {
        Q_DECLARE_PUBLIC(WASAPIAudioDriver)
    public:
        WASAPIAudioDriver::ShareMode mode = WASAPIAudioDriver::Shared;
        bool ownsCom = false;
        Microsoft::WRL::ComPtr<IMMDeviceEnumerator> enumerator;
        WASAPIEndpointNotification *notification = nullptr;
        QTimer *refreshTimer = nullptr;
        QList<WASAPIEndpoint> endpoints;
    };

}

#endif // TALCS_WASAPIAUDIODRIVER_P_H
