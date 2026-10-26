// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TALCS_COREAUDIOUTILS_P_H
#define TALCS_COREAUDIOUTILS_P_H

#include <chrono>
#include <cstddef>
#include <limits>
#include <vector>

#include <QByteArray>
#include <QCoreApplication>
#include <QString>

#include <CoreAudio/CoreAudio.h>
#include <CoreFoundation/CoreFoundation.h>

namespace talcs {

    inline qint64 coreAudioEventTime() {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    }

    inline AudioObjectPropertyAddress coreAudioPropertyAddress(AudioObjectPropertySelector selector, AudioObjectPropertyScope scope = kAudioObjectPropertyScopeGlobal) {
        return {selector, scope, kAudioObjectPropertyElementMain};
    }

    template <class T>
    OSStatus coreAudioReadProperty(AudioObjectID object, AudioObjectPropertySelector selector, T &value, AudioObjectPropertyScope scope = kAudioObjectPropertyScopeGlobal) {
        const auto address = coreAudioPropertyAddress(selector, scope);
        UInt32 size = sizeof(T);
        const auto result = AudioObjectGetPropertyData(object, &address, 0, nullptr, &size, &value);
        return result == noErr && size != sizeof(T) ? kAudioHardwareBadPropertySizeError : result;
    }

    template <class T>
    OSStatus coreAudioWriteProperty(AudioObjectID object, AudioObjectPropertySelector selector, const T &value) {
        const auto address = coreAudioPropertyAddress(selector);
        return AudioObjectSetPropertyData(object, &address, 0, nullptr, sizeof(T), &value);
    }

    template <class T>
    OSStatus coreAudioReadArray(AudioObjectID object, AudioObjectPropertySelector selector, std::vector<T> &values, AudioObjectPropertyScope scope = kAudioObjectPropertyScopeGlobal) {
        const auto address = coreAudioPropertyAddress(selector, scope);
        UInt32 size = 0;
        auto result = AudioObjectGetPropertyDataSize(object, &address, 0, nullptr, &size);
        if (result != noErr) {
            return result;
        }
        if (size % sizeof(T)) {
            return kAudioHardwareBadPropertySizeError;
        }
        values.resize(size / sizeof(T));
        if (!size) {
            return noErr;
        }
        const auto capacity = size;
        result = AudioObjectGetPropertyData(object, &address, 0, nullptr, &size, values.data());
        if (result == noErr) {
            if (size > capacity || size % sizeof(T)) {
                return kAudioHardwareBadPropertySizeError;
            }
            values.resize(size / sizeof(T));
        }
        return result;
    }

    inline OSStatus coreAudioReadString(AudioObjectID object, AudioObjectPropertySelector selector, QString &text) {
        CFStringRef value = nullptr;
        const auto result = coreAudioReadProperty(object, selector, value);
        if (result == noErr && value) {
            text = QString::fromCFString(value);
        }
        if (value) {
            CFRelease(value);
        }
        return result;
    }

    inline OSStatus coreAudioOutputChannelCount(AudioDeviceID device, int &channels) {
        const auto address = coreAudioPropertyAddress(kAudioDevicePropertyStreamConfiguration, kAudioDevicePropertyScopeOutput);
        UInt32 size = 0;
        auto result = AudioObjectGetPropertyDataSize(device, &address, 0, nullptr, &size);
        if (result != noErr) {
            return result;
        }
        if (size < offsetof(AudioBufferList, mBuffers)) {
            return kAudioHardwareBadPropertySizeError;
        }
        QByteArray storage(size, '\0');
        const auto capacity = size;
        result = AudioObjectGetPropertyData(device, &address, 0, nullptr, &size, storage.data());
        if (result != noErr) {
            return result;
        }
        const auto *buffers = reinterpret_cast<const AudioBufferList *>(storage.constData());
        if (size > capacity || size < offsetof(AudioBufferList, mBuffers) || buffers->mNumberBuffers > (size - offsetof(AudioBufferList, mBuffers)) / sizeof(::AudioBuffer)) {
            return kAudioHardwareBadPropertySizeError;
        }
        quint64 total = 0;
        for (UInt32 i = 0; i < buffers->mNumberBuffers; ++i) {
            total += buffers->mBuffers[i].mNumberChannels;
        }
        if (total > std::numeric_limits<int>::max()) {
            return kAudioHardwareUnsupportedOperationError;
        }
        channels = static_cast<int>(total);
        return noErr;
    }

    inline QString coreAudioErrorString(OSStatus result) {
        const auto code = QStringLiteral("0x%1").arg(static_cast<quint32>(result), 8, 16, QLatin1Char('0'));
        const char *message = nullptr;
        switch (result) {
            case kAudioHardwareBadDeviceError:
            case kAudioHardwareBadObjectError:
                message = QT_TRANSLATE_NOOP("talcs::CoreAudioAudioDevice", "The audio device is no longer available.");
                break;
            case kAudioHardwareIllegalOperationError:
            case kAudioDevicePermissionsError:
                message = QT_TRANSLATE_NOOP("talcs::CoreAudioAudioDevice", "The audio device does not permit the requested operation.");
                break;
            case kAudioDeviceUnsupportedFormatError:
                message = QT_TRANSLATE_NOOP("talcs::CoreAudioAudioDevice", "The requested audio format is not supported.");
                break;
            case kAudioHardwareUnsupportedOperationError:
            case kAudioHardwareUnknownPropertyError:
                message = QT_TRANSLATE_NOOP("talcs::CoreAudioAudioDevice", "The audio device does not support the requested operation.");
                break;
            default:
                break;
        }
        if (message) {
            return QStringLiteral("%1 (%2)").arg(QCoreApplication::translate("talcs::CoreAudioAudioDevice", message), code);
        }
        return QCoreApplication::translate("talcs::CoreAudioAudioDevice", "The audio operation failed (%1).").arg(code);
    }

}

#endif // TALCS_COREAUDIOUTILS_P_H
