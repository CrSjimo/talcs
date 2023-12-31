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

#include "ASIOAudioDriver.h"
#include "ASIOAudioDriver_p.h"

#include "ASIOAudioDevice.h"

#include <QFileInfo>
#include <QDebug>

namespace talcs {

    /**
     * @class ASIOAudioDriver
     * @brief The audio driver using ASIO
     * @see @link URL https://forums.steinberg.net/c/developer/asio/ @endlink
     */

    /**
     * Constructor.
     */
    ASIOAudioDriver::ASIOAudioDriver(QObject * parent) : AudioDriver(*new ASIOAudioDriverPrivate, parent) {
        setName("ASIO");
    }

    /**
     * Destructor.
     *
     * If the driver is still running, it will be terminated now.
     */
    ASIOAudioDriver::~ASIOAudioDriver() {
        ASIOAudioDriver::finalize();
    }

    static bool checkDriverCOMClass(char *clsidStr) {
        CharLowerBuffA(clsidStr, std::strlen(clsidStr));
        HKEY hkeyClsid;
        HKEY hkeySub, hKeyInproc;
        if (::RegOpenKeyExA(HKEY_CLASSES_ROOT, "clsid", 0, KEY_READ, &hkeyClsid) != ERROR_SUCCESS)
            return false;
        int index = 0;
        char buf[512];
        bool result = false;
        while (::RegEnumKeyA(hkeyClsid, index++, buf, MAXPATHLEN) == ERROR_SUCCESS) {
            CharLowerBuffA(buf, std::strlen(buf));
            if (std::strcmp(clsidStr, buf) != 0)
                continue;

            // Firstly get DLL path from .\<CLSID>\InprocServer32\<Default>
            if (::RegOpenKeyExA(hkeyClsid, buf, 0, KEY_READ, &hkeySub) != ERROR_SUCCESS)
                goto finally1;
            if (::RegOpenKeyExA(hkeySub, "InprocServer32", 0, KEY_READ, &hKeyInproc) != ERROR_SUCCESS)
                goto finally2;
            DWORD valueType = REG_SZ;
            DWORD valueSize = 512;
            if (::RegQueryValueEx(hKeyInproc, NULL, NULL, &valueType, (LPBYTE) buf, &valueSize) != ERROR_SUCCESS)
                goto finally3;

            // Then check the existence of DLL file
            result = QFileInfo::exists(buf);
            break;
        }
    finally3:
        ::RegCloseKey(hKeyInproc);
    finally2:
        ::RegCloseKey(hkeySub);
    finally1:
        ::RegCloseKey(hkeyClsid);
        return result;
    }

    void ASIOAudioDriverPrivate::createDriverSpec(HKEY hkey, char *keyName) {
        HKEY hkeySub;

        // Open the item of driver in the Registry
        if (::RegOpenKeyExA(hkey, keyName, 0, KEY_READ, &hkeySub) != ERROR_SUCCESS)
            return;

        char buf[256];
        WCHAR wStr[128];
        DWORD valueType;
        DWORD valueSize;
        CLSID clsid;
        QString driverName;

        // Get CLSID
        valueType = REG_SZ;
        valueSize = 256;
        if (::RegQueryValueExA(hkeySub, "clsid", NULL, &valueType, (LPBYTE) buf, &valueSize) != ERROR_SUCCESS)
            goto finally;
        if (!checkDriverCOMClass(buf))
            goto finally;
        ::MultiByteToWideChar(CP_ACP, 0, buf, -1, wStr, 128);
        if (::CLSIDFromString(wStr, &clsid) != S_OK)
            goto finally;

        // Get driver name
        valueType = REG_SZ;
        valueSize = 256;
        if (::RegQueryValueExA(hkeySub, "description", NULL, &valueType, (LPBYTE) buf, &valueSize) == ERROR_SUCCESS) {
            driverName = buf;
        } else {
            driverName = keyName;
        }

        asioDriverSpecs.append({clsid, driverName});

    finally:
        ::RegCloseKey(hkeySub);
    }

    bool ASIOAudioDriver::initialize() {
        Q_D(ASIOAudioDriver);

        d->asioDriverSpecs.clear();

        HKEY hkEnum = nullptr;
        char keyname[MAXPATHLEN];

        if (::RegOpenKeyA(HKEY_LOCAL_MACHINE, "software\\asio", &hkEnum) != ERROR_SUCCESS)
            return false;
        int index = 0;
        while (::RegEnumKeyA(hkEnum, index++, keyname, MAXPATHLEN) == ERROR_SUCCESS) {
            d->createDriverSpec(hkEnum, keyname);
        }
        if (!d->asioDriverSpecs.isEmpty())
            ::CoInitialize(NULL);
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
            if (::CoCreateInstance(spec.clsid, NULL, CLSCTX_INPROC_SERVER, spec.clsid, &iasio) != S_OK)
                return nullptr;
            return new ASIOAudioDevice(name, (IASIO *) iasio, this);
        }
        return nullptr;
    }
}
