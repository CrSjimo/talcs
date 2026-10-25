// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TALCS_WASAPIAUDIOUTILS_P_H
#define TALCS_WASAPIAUDIOUTILS_P_H

#include <QCoreApplication>
#include <QString>
#include <qt_windows.h>

#include <audioclient.h>
#include <mmdeviceapi.h>
#include <wrl/client.h>

namespace talcs {

    inline QString wasapiErrorString(HRESULT result) {
        const auto systemMessage = [](DWORD messageId) {
            wchar_t *message = nullptr;
            const auto length = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr, messageId, 0, reinterpret_cast<wchar_t *>(&message), 0, nullptr);
            const auto text = length ? QString::fromWCharArray(message, static_cast<int>(length)).trimmed() : QString();
            LocalFree(message);
            return text;
        };
        const auto code = QStringLiteral("0x%1").arg(static_cast<quint32>(result), 8, 16, QLatin1Char('0'));
        auto detail = systemMessage(static_cast<DWORD>(result));
        if (detail.isEmpty() && HRESULT_FACILITY(result) == FACILITY_WIN32) {
            detail = systemMessage(HRESULT_CODE(result));
        }
        if (!detail.isEmpty()) {
            return QStringLiteral("%1 (%2)").arg(detail, code);
        }
        switch (result) {
            case AUDCLNT_E_DEVICE_INVALIDATED:
                detail = QCoreApplication::translate("talcs::WASAPIAudioDevice", "The audio device is no longer available.");
                break;
            case AUDCLNT_E_RESOURCES_INVALIDATED:
                detail = QCoreApplication::translate("talcs::WASAPIAudioDevice", "The audio stream is no longer available.");
                break;
            case AUDCLNT_E_DEVICE_IN_USE:
                detail = QCoreApplication::translate("talcs::WASAPIAudioDevice", "The audio device is in use by another application.");
                break;
            case AUDCLNT_E_EXCLUSIVE_MODE_NOT_ALLOWED:
                detail = QCoreApplication::translate("talcs::WASAPIAudioDevice", "Exclusive access is disabled for this audio device.");
                break;
            case AUDCLNT_E_UNSUPPORTED_FORMAT:
                detail = QCoreApplication::translate("talcs::WASAPIAudioDevice", "The requested audio format is not supported.");
                break;
            case AUDCLNT_E_INVALID_DEVICE_PERIOD:
            case AUDCLNT_E_BUFFER_SIZE_ERROR:
                detail = QCoreApplication::translate("talcs::WASAPIAudioDevice", "The requested buffer size is not supported at this sample rate.");
                break;
            case AUDCLNT_E_ENGINE_FORMAT_LOCKED:
                detail = QCoreApplication::translate("talcs::WASAPIAudioDevice", "Another application has locked the shared audio format.");
                break;
            case AUDCLNT_E_ENGINE_PERIODICITY_LOCKED:
                detail = QCoreApplication::translate("talcs::WASAPIAudioDevice", "Another application has locked the shared audio buffer period.");
                break;
            case AUDCLNT_E_SERVICE_NOT_RUNNING:
                detail = QCoreApplication::translate("talcs::WASAPIAudioDevice", "The Windows audio service is not running.");
                break;
            default:
                break;
        }
        if (detail.isEmpty()) {
            return QCoreApplication::translate("talcs::WASAPIAudioDevice", "The audio operation failed (%1).").arg(code);
        }
        return QStringLiteral("%1 (%2)").arg(detail, code);
    }

    inline QString wasapiEndpointId(IMMDevice *device, HRESULT *error = nullptr) {
        LPWSTR id = nullptr;
        const auto status = device->GetId(&id);
        if (error) {
            *error = status;
        }
        if (FAILED(status)) {
            return {};
        }
        const auto result = QString::fromWCharArray(id);
        CoTaskMemFree(id);
        return result;
    }

}

#endif // TALCS_WASAPIAUDIOUTILS_P_H
