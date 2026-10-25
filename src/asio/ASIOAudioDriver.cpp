/*******************************************************************************
 * Copyright (c) 2023 CrSjimo                                                  *
 * SPDX-License-Identifier: GPL-3.0-only                                       *
 *                                                                             *
 * This file is part of TALCS.                                                 *
 *                                                                             *
 * TALCS is free software: you can redistribute it and/or modify it under the  *
 * terms of the GNU General Public License as published by the Free Software   *
 * Foundation, version 3 of the License.                                       *
 *                                                                             *
 * TALCS is distributed in the hope that it will be useful, but WITHOUT ANY    *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS   *
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more       *
 * details.                                                                    *
 *                                                                             *
 * You should have received a copy of the GNU General Public License along     *
 * with TALCS. If not, see <https://www.gnu.org/licenses/>.                    *
 ******************************************************************************/

#include "ASIOAudioDriver.h"
#include "ASIOAudioDriver_p.h"

#include <array>
#include <memory>
#include <objbase.h>
#include <type_traits>
#include <vector>

#include <QDebug>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QThread>

#include <TalcsAsio/private/ASIOAudioDevice_p.h>

namespace talcs {

    Q_LOGGING_CATEGORY(lcASIOAudioDriver, "talcs.asio.driver")

    QString asioSystemErrorString(DWORD error) {
        wchar_t *message = nullptr;
        auto length = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr, error, 0, reinterpret_cast<wchar_t *>(&message), 0, nullptr);
        if (!length && HRESULT_FACILITY(error) == FACILITY_WIN32) {
            length = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr, HRESULT_CODE(error), 0, reinterpret_cast<wchar_t *>(&message), 0, nullptr);
        }
        const auto detail = length ? QString::fromWCharArray(message, static_cast<int>(length)).trimmed() : QString();
        LocalFree(message);
        return QStringLiteral("%1 (0x%2)").arg(detail).arg(error, 8, 16, QLatin1Char('0'));
    }

    struct RegistryKeyCloser {
        void operator()(HKEY key) const {
            if (key) {
                RegCloseKey(key);
            }
        }
    };

    using RegistryKey = std::unique_ptr<std::remove_pointer_t<HKEY>, RegistryKeyCloser>;

    static LSTATUS readRegistryString(HKEY key, const wchar_t *name, QString &value) {
        constexpr DWORD flags = RRF_RT_REG_SZ | RRF_RT_REG_EXPAND_SZ;
        DWORD bytes = 0;
        auto result = RegGetValueW(key, nullptr, name, flags, nullptr, nullptr, &bytes);
        for (int attempt = 0; attempt < 3 && (result == ERROR_SUCCESS || result == ERROR_MORE_DATA); attempt++) {
            if (bytes % sizeof(wchar_t) != 0) {
                return ERROR_INVALID_DATA;
            }
            std::vector<wchar_t> buffer(bytes / sizeof(wchar_t) + 1, L'\0');
            result = RegGetValueW(key, nullptr, name, flags, nullptr, buffer.data(), &bytes);
            if (result == ERROR_SUCCESS) {
                value = QString::fromWCharArray(buffer.data());
                return ERROR_SUCCESS;
            }
        }
        return result;
    }

    void ASIOAudioDriverPrivate::createDriverSpec(HKEY key, const QString &keyName) {
        HKEY rawKey = nullptr;
        auto result = RegOpenKeyExW(key, reinterpret_cast<const wchar_t *>(keyName.utf16()), 0, KEY_READ, &rawKey);
        if (result != ERROR_SUCCESS) {
            qCWarning(lcASIOAudioDriver) << "ASIO device registry entry open failed." << "registry entry:" << keyName << "error:" << asioSystemErrorString(result);
            return;
        }
        RegistryKey deviceKey(rawKey);
        QString clsidText;
        result = readRegistryString(deviceKey.get(), L"clsid", clsidText);
        if (result != ERROR_SUCCESS) {
            qCWarning(lcASIOAudioDriver) << "ASIO device CLSID retrieval failed." << "registry entry:" << keyName << "error:" << asioSystemErrorString(result);
            return;
        }
        CLSID clsid{};
        const auto comResult = CLSIDFromString(reinterpret_cast<const wchar_t *>(clsidText.utf16()), &clsid);
        if (FAILED(comResult)) {
            qCWarning(lcASIOAudioDriver) << "ASIO device CLSID parsing failed." << "registry entry:" << keyName << "error:" << asioSystemErrorString(comResult);
            return;
        }

        const auto serverKeyName = QStringLiteral("CLSID\\%1\\InprocServer32").arg(clsidText);
        rawKey = nullptr;
        result = RegOpenKeyExW(HKEY_CLASSES_ROOT, reinterpret_cast<const wchar_t *>(serverKeyName.utf16()), 0, KEY_READ, &rawKey);
        if (result != ERROR_SUCCESS) {
            qCWarning(lcASIOAudioDriver) << "ASIO COM server registry open failed." << "CLSID:" << clsidText << "error:" << asioSystemErrorString(result);
            return;
        }
        RegistryKey serverKey(rawKey);
        QString serverPath;
        result = readRegistryString(serverKey.get(), nullptr, serverPath);
        if (result != ERROR_SUCCESS) {
            qCWarning(lcASIOAudioDriver) << "ASIO COM server validation failed." << "CLSID:" << clsidText << "error:" << asioSystemErrorString(result);
            return;
        }
        if (!QFileInfo::exists(serverPath)) {
            qCWarning(lcASIOAudioDriver) << "ASIO COM server DLL does not exist." << "CLSID:" << clsidText << "DLL:" << serverPath;
            return;
        }

        QString driverName;
        if (readRegistryString(deviceKey.get(), L"description", driverName) != ERROR_SUCCESS || driverName.isEmpty()) {
            driverName = keyName;
        }
        asioDriverSpecs.append({clsid, driverName});
        qCInfo(lcASIOAudioDriver) << "ASIO audio device enumerated." << "device:" << driverName;
    }

    ASIOAudioDriver::ASIOAudioDriver(QObject *parent) : AudioDriver(*new ASIOAudioDriverPrivate, parent) {
        setName(QString::fromUtf8(QT_TRANSLATE_NOOP("talcs::AudioDriver", "ASIO")));
    }

    ASIOAudioDriver::~ASIOAudioDriver() {
        ASIOAudioDriver::finalize();
    }

    bool ASIOAudioDriver::initialize() {
        Q_D(ASIOAudioDriver);
        if (QThread::currentThread() != thread()) {
            setErrorString(tr("The ASIO driver must be initialized on its owning thread."));
            return false;
        }
        if (isInitialized()) {
            if (d->comThreadId == GetCurrentThreadId()) {
                return true;
            }
            setErrorString(tr("The ASIO driver must remain on its COM initialization thread."));
            return false;
        }
        d->asioDriverSpecs.clear();
        HKEY rawKey = nullptr;
        auto result = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\ASIO", 0, KEY_READ, &rawKey);
        if (result != ERROR_SUCCESS) {
            setErrorString(asioSystemErrorString(result));
            return false;
        }
        RegistryKey asioKey(rawKey);
        for (DWORD index = 0;; index++) {
            std::array<wchar_t, 256> keyName{};
            DWORD length = static_cast<DWORD>(keyName.size());
            result = RegEnumKeyExW(asioKey.get(), index, keyName.data(), &length, nullptr, nullptr, nullptr, nullptr);
            if (result == ERROR_NO_MORE_ITEMS) {
                break;
            }
            if (result != ERROR_SUCCESS) {
                setErrorString(asioSystemErrorString(result));
                d->asioDriverSpecs.clear();
                return false;
            }
            d->createDriverSpec(asioKey.get(), QString::fromWCharArray(keyName.data(), static_cast<int>(length)));
        }
        if (d->asioDriverSpecs.isEmpty()) {
            setErrorString(tr("No ASIO audio drivers are available."));
            return false;
        }

        const auto comResult = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        if (FAILED(comResult) && comResult != RPC_E_CHANGED_MODE) {
            setErrorString(asioSystemErrorString(comResult));
            return false;
        }
        d->ownsCom = SUCCEEDED(comResult);
        d->comThreadId = GetCurrentThreadId();
        // The hidden top-level window supplies a stable owner for driver dialogs.
        d->systemHandle = CreateWindowExW(0, L"STATIC", L"TALCS ASIO", WS_OVERLAPPED,
            0, 0, 0, 0, nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
        if (!d->systemHandle) {
            setErrorString(asioSystemErrorString(GetLastError()));
            finalize();
            return false;
        }
        setErrorString({});
        qCInfo(lcASIOAudioDriver) << "ASIO driver initialization completed." << "devices:" << devices();
        return AudioDriver::initialize();
    }

    void ASIOAudioDriver::finalize() {
        Q_D(ASIOAudioDriver);
        if (d->comThreadId && d->comThreadId != GetCurrentThreadId()) {
            qCWarning(lcASIOAudioDriver) << "ASIO driver finalization requires its COM initialization thread.";
            return;
        }
        for (auto device : findChildren<ASIOAudioDevice *>(QString(), Qt::FindDirectChildrenOnly)) {
            device->shutdown();
        }
        if (d->systemHandle) {
            DestroyWindow(d->systemHandle);
            d->systemHandle = nullptr;
        }
        if (d->ownsCom) {
            CoUninitialize();
            d->ownsCom = false;
        }
        d->comThreadId = 0;
        d->asioDriverSpecs.clear();
        AudioDriver::finalize();
    }

    QStringList ASIOAudioDriver::devices() const {
        Q_D(const ASIOAudioDriver);
        QStringList list;
        for (const auto &spec : d->asioDriverSpecs) {
            list.append(spec.driverName);
        }
        return list;
    }

    QString ASIOAudioDriver::defaultDevice() const {
        return AudioDriver::defaultDevice();
    }

    AudioDevice *ASIOAudioDriver::createDevice(const QString &name) {
        Q_D(ASIOAudioDriver);
        if (!isInitialized() || d->comThreadId != GetCurrentThreadId()) {
            setErrorString(tr("The ASIO driver must be initialized on the current thread before creating a device."));
            return nullptr;
        }
        for (const auto &spec : d->asioDriverSpecs) {
            if (spec.driverName == name) {
                return new ASIOAudioDevice(name, spec.clsid, d->systemHandle, this);
            }
        }
        setErrorString(tr("The requested ASIO audio device is not available."));
        return nullptr;
    }
}
