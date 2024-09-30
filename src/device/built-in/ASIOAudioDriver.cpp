/******************************************************************************
 * Copyright (c) 2023 CrSjimo                                                 *
 *                                                                            *
 * This file is part of TALCS.                                                *
 *                                                                            *
 * TALCS is free software: you can redistribute it and/or modify it under the *
 * terms of the GNU Lesser General Public License as published by the Free    *
 * Software Foundation, either version 3 of the License, or (at your option)  *
 * any later version.                                                         *
 *                                                                            *
 * TALCS is distributed in the hope that it will be useful, but WITHOUT ANY   *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS  *
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for    *
 * more details.                                                              *
 *                                                                            *
 * You should have received a copy of the GNU Lesser General Public License   *
 * along with TALCS. If not, see <https://www.gnu.org/licenses/>.             *
 ******************************************************************************/

#include "ASIOAudioDriver_p.h"

#include <QDebug>
#include <QFileInfo>

#include "ASIOAudioDevice_p.h"

namespace talcs {

    ASIOAudioDriver::ASIOAudioDriver(QObject * parent) : AudioDriver(*new ASIOAudioDriverPrivate, parent) {
        setName("ASIO");
    }

    ASIOAudioDriver::~ASIOAudioDriver() {
        ASIOAudioDriver::finalize();
    }

    struct keyClose {
        void operator()(HKEY hKey) const {
            if (hKey)
                ::RegCloseKey(hKey);
        }
    };

    using HKEYUniquePtr = std::unique_ptr<std::remove_pointer_t<HKEY>, keyClose>;

    static bool checkDriverCOMClass(char *clsidStr) {
        CharLowerBuffA(clsidStr, std::strlen(clsidStr));
        HKEYUniquePtr hKeyClsid, // HKEY_CLASSES_ROOT\clsid
                      hKeySub, // HKEY_CLASSES_ROOT\clsid\<CLSID>
                      hKeyInproc; // HKEY_CLASSES_ROOT\clsid\<CLSID>\InprocServer32
        HKEY hKey_;
        if (::RegOpenKeyExA(HKEY_CLASSES_ROOT, "clsid", 0, KEY_READ, &hKey_) != ERROR_SUCCESS)
            return false;
        hKeyClsid.reset(hKey_);
        int index = 0;
        char buf[512];
        while (::RegEnumKeyA(hKeyClsid.get(), index++, buf, MAXPATHLEN) == ERROR_SUCCESS) {
            CharLowerBuffA(buf, std::strlen(buf));
            if (std::strcmp(clsidStr, buf) != 0)
                continue;

            // Firstly get DLL path from .\<CLSID>\InprocServer32\<Default>
            if (::RegOpenKeyExA(hKeyClsid.get(), buf, 0, KEY_READ, &hKey_) != ERROR_SUCCESS)
                return false;
            hKeySub.reset(hKey_);
            if (::RegOpenKeyExA(hKeySub.get(), "InprocServer32", 0, KEY_READ, &hKey_) != ERROR_SUCCESS)
                return false;
            hKeyInproc.reset(hKey_);
            DWORD valueType = REG_SZ;
            DWORD valueSize = 512;
            if (::RegQueryValueExA(hKeyInproc.get(), nullptr, nullptr, &valueType, (LPBYTE) buf, &valueSize) != ERROR_SUCCESS)
                return false;

            // Then check the existence of DLL file
            return QFileInfo::exists(buf);
        }
        return false;
    }

    void ASIOAudioDriverPrivate::createDriverSpec(HKEY hkey, const char *keyName) {
        HKEYUniquePtr hKeySub; // HEKY_LOCAL_MACHINE\software\asio\<driver>
        HKEY hKey_;

        // Open the item of driver in the Registry
        if (::RegOpenKeyExA(hkey, keyName, 0, KEY_READ, &hKey_) != ERROR_SUCCESS)
            return;
        hKeySub.reset(hKey_);

        char buf[256];
        WCHAR wBuf[128];
        DWORD valueType;
        DWORD valueSize;
        CLSID clsid;
        QString driverName;

        // Get CLSID
        valueType = REG_SZ;
        valueSize = 256;
        if (::RegQueryValueExA(hKeySub.get(), "clsid", nullptr, &valueType, (LPBYTE) buf, &valueSize) != ERROR_SUCCESS)
            return;
        if (!checkDriverCOMClass(buf))
            return;
        ::MultiByteToWideChar(CP_ACP, 0, buf, -1, wBuf, 128);
        if (::CLSIDFromString(wBuf, &clsid) != S_OK)
            return;

        // Get driver name
        valueType = REG_SZ;
        valueSize = 256;
        if (::RegQueryValueExA(hKeySub.get(), "description", nullptr, &valueType, (LPBYTE) buf, &valueSize) == ERROR_SUCCESS) {
            driverName = buf;
        } else {
            driverName = keyName;
        }

        asioDriverSpecs.append({clsid, driverName});
    }

    bool ASIOAudioDriver::initialize() {
        Q_D(ASIOAudioDriver);

        d->asioDriverSpecs.clear();

        HKEYUniquePtr hKeyEnum; // HEKY_LOCAL_MACHINE\software\asio
        HKEY hKey_;
        char keyname[MAXPATHLEN];

        if (::RegOpenKeyA(HKEY_LOCAL_MACHINE, "software\\asio", &hKey_) != ERROR_SUCCESS)
            return false;
        hKeyEnum.reset(hKey_);
        int index = 0;
        while (::RegEnumKeyA(hKeyEnum.get(), index++, keyname, MAXPATHLEN) == ERROR_SUCCESS) {
            d->createDriverSpec(hKeyEnum.get(), keyname);
        }
        if (!d->asioDriverSpecs.isEmpty())
            ::CoInitialize(nullptr);
        return AudioDriver::initialize();
    }
    void ASIOAudioDriver::finalize() {
        ::CoUninitialize();
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
        for (const auto &spec : d->asioDriverSpecs) {
            if (spec.driverName != name)
                continue;
            void *iasio;
            if (::CoCreateInstance(spec.clsid, nullptr, CLSCTX_INPROC_SERVER, spec.clsid, &iasio) != S_OK)
                return nullptr;
            return new ASIOAudioDevice(name, (IASIO *) iasio, this);
        }
        return nullptr;
    }
}
