// SPDX-FileCopyrightText: 2026 CrSjimo
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "NativeAudioDevice_p.h"
#include "NativeAudioDevice_p_p.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include <TalcsCore/AudioSource.h>

namespace talcs {

    NativeAudioDevice::NativeAudioDevice(NativeAudioDevicePrivate &d, QObject *parent) : AudioDevice(d, parent) {
        d.watchdog = new QTimer(this);
        d.watchdog->setInterval(100);
        connect(d.watchdog, &QTimer::timeout, this, [this] {
            Q_D(NativeAudioDevice);
            if (const auto error = d->streamError.load()) {
                const auto message = streamErrorString(error);
                close();
                d->streamError = 0;
                setErrorString(message);
                Q_EMIT closed();
            }
        });
    }

    NativeAudioDevice::~NativeAudioDevice() = default;

    bool NativeAudioDevice::configure(qint64 bufferSize, double sampleRate) {
        Q_D(NativeAudioDevice);
        if (bufferSize <= 0 || bufferSize > 1048576 || !std::isfinite(sampleRate) || sampleRate <= 0 || sampleRate != std::floor(sampleRate) || sampleRate > std::numeric_limits<int>::max() || activeChannelCount() <= 0) {
            setErrorString(tr("The requested audio format is invalid."));
            return false;
        }
        d->streamError = 0;
        setErrorString({});
        return AudioStreamBase::open(bufferSize, sampleRate);
    }

    bool NativeAudioDevice::start(AudioDeviceCallback *callback) {
        Q_D(NativeAudioDevice);
        if (!isOpen() || !callback) {
            setErrorString(tr("The audio device is not open or the audio callback is invalid."));
            return false;
        }
        stop();
        if (!callback->deviceWillStartCallback(this)) {
            setErrorString(tr("The audio callback could not be initialized."));
            return false;
        }
        {
            QMutexLocker locker(&d->callbackMutex);
            d->callback = callback;
        }
        if (!activateStream()) {
            {
                QMutexLocker locker(&d->callbackMutex);
                d->callback = nullptr;
            }
            deactivateStream();
            callback->deviceStoppedCallback();
            return false;
        }
        d->watchdog->start();
        return AudioDevice::start(callback);
    }

    void NativeAudioDevice::stop() {
        Q_D(NativeAudioDevice);
        d->watchdog->stop();
        AudioDeviceCallback *callback;
        {
            QMutexLocker locker(&d->callbackMutex);
            callback = d->callback;
            d->callback = nullptr;
        }
        deactivateStream();
        AudioDevice::stop();
        if (callback) {
            callback->deviceStoppedCallback();
        }
    }

    void NativeAudioDevice::lock() {
        Q_D(NativeAudioDevice);
        d->callbackMutex.lock();
    }

    void NativeAudioDevice::unlock() {
        Q_D(NativeAudioDevice);
        d->callbackMutex.unlock();
    }

    void NativeAudioDevice::render(float *output, qint64 frames) {
        Q_D(NativeAudioDevice);
        QMutexLocker locker(&d->callbackMutex);
        const auto channels = activeChannelCount();
        std::fill_n(output, frames * channels, 0.0f);
        const auto maximumFrames = bufferSize();
        if (!d->callback || d->streamError.load() || maximumFrames <= 0) {
            return;
        }
        // Sources allocate their processing buffers using the size supplied before playback starts.
        for (qint64 position = 0; position < frames && !d->streamError.load();) {
            const auto count = std::min(maximumFrames, frames - position);
            d->interleavedBuffer.reset(output + position * channels, channels, count);
            d->callback->workCallback({&d->interleavedBuffer, 0, count});
            position += count;
        }
    }

    void NativeAudioDevice::render(float *const *output, qint64 frames) {
        Q_D(NativeAudioDevice);
        QMutexLocker locker(&d->callbackMutex);
        for (int channel = 0; channel < activeChannelCount(); ++channel) {
            std::fill_n(output[channel], frames, 0.0f);
        }
        if (!d->callback || d->streamError.load()) {
            return;
        }
        d->planarBuffer.reset(output, activeChannelCount(), frames);
        d->callback->workCallback({&d->planarBuffer, 0, frames});
    }

    void NativeAudioDevice::fail(int error) {
        Q_D(NativeAudioDevice);
        int expected = 0;
        d->streamError.compare_exchange_strong(expected, error ? error : -1);
    }

    QString NativeAudioDevice::streamErrorString(int error) const {
        return tr("The audio output stream failed (error %L1).").arg(error);
    }

}
