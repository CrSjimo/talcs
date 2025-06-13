/******************************************************************************
 * Copyright (c) 2023-2025 CrSjimo                                            *
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

#include "SoundIOAudioDevice_p.h"

#include <algorithm>

#include <QDebug>

#include <TalcsCore/AudioSource.h>
#include <TalcsCore/IAudioSampleContainer.h>
#include <TalcsDevice/AudioDeviceCallback.h>

#include "SoundIOAudioDriver_p.h"

namespace talcs {

    static const QList<double> COMMON_SAMPLE_RATES = {8000,   11025,  12000,  16000,  22050,  24000,
                                                      32000,  44100,  48000,  64000,  88200,  96000,
                                                      128000, 176400, 192000, 256000, 352800, 384000};

    static const QList<qint64> COMMON_BUFFER_SIZES = {512, 1024, 2048, 4096, 8192};

    static constexpr qint8 floatToInt8(float v) {
        return static_cast<qint8>(v * (static_cast<double>(0x7fL) + 0.4999999999999999));
    }
    static constexpr qint16 floatToInt16(float v) {
        return static_cast<qint16>(v * (static_cast<double>(0x7fffL) + 0.4999999999999999));
    }
    static constexpr qint32 floatToInt24(float v) {
        return static_cast<qint32>(v * (static_cast<double>(0x7fffffL) + 0.4999999999999999));
    }
    static constexpr qint32 floatToInt32(float v) {
        return static_cast<qint32>(v * (static_cast<double>(0x7fffffffL) + 0.4999999999999999));
    }
    static constexpr float int8ToFloat(qint8 v) {
        return static_cast<float>(v / 127.0);
    }
    static constexpr float int16ToFloat(qint16 v) {
        return static_cast<float>(v / 32767.0);
    }
    static constexpr float int24ToFloat(qint32 v) {
        return static_cast<float>(v / 8388607.0);
    }
    static constexpr float int32ToFloat(qint32 v) {
        return static_cast<float>(v / 2147483647.0);
    }

    class SoundIOChannelAreaWrapper : public IAudioSampleContainer {
        SoundIoChannelArea *m_area;
        SoundIoFormat m_format;
        int m_channelCount;
        bool m_isContinuous;
        qint64 m_sampleCount;
    public:
        SoundIOChannelAreaWrapper(SoundIoChannelArea *area, SoundIoFormat format, int channelCount, qint64 sampleCount) : m_area(area), m_format(SoundIoFormatS16LE), m_channelCount(channelCount), m_sampleCount(sampleCount) {
            m_isContinuous = format == SoundIoFormatFloat32LE && std::all_of(area, area + channelCount, [](const auto &a) { return a.step == sizeof(float); });
        }
        ~SoundIOChannelAreaWrapper() override = default;
        float sample(int channel, qint64 pos) const override {
            auto p = m_area[channel].ptr + pos * m_area->step;
            switch(m_format) {
                case SoundIoFormatFloat32LE:
                    return *reinterpret_cast<float *>(p);
                case SoundIoFormatU8:
                    return int8ToFloat(*reinterpret_cast<qint8 *>(p));
                case SoundIoFormatS16LE:
                    return int16ToFloat(*reinterpret_cast<qint16 *>(p));
                case SoundIoFormatS24LE:
                    return int24ToFloat(*reinterpret_cast<qint32 *>(p));
                case SoundIoFormatS32LE:
                    return int32ToFloat(*reinterpret_cast<qint32 *>(p));
                case SoundIoFormatFloat64LE:
                    return static_cast<float>(*reinterpret_cast<double *>(p));
                default:
                    Q_UNREACHABLE();
            }
        }
        bool isContinuous() const override {
            return m_isContinuous;
        }
        const float *readPointerTo(int channel, qint64 startPos) const override {
            if (isContinuous()) {
                return reinterpret_cast<float *>(m_area[channel].ptr + startPos * m_area[channel].step);
            }
            return nullptr;
        }
        int channelCount() const override {
            return m_channelCount;
        }
        qint64 sampleCount() const override {
            return m_sampleCount;
        }
        void setSample(int channel, qint64 pos, float value) override {
            auto p = m_area[channel].ptr + pos * m_area->step;
            switch(m_format) {
                case SoundIoFormatFloat32LE:
                    *reinterpret_cast<float *>(p) = value;
                    break;
                case SoundIoFormatU8:
                    *reinterpret_cast<qint8 *>(p) = floatToInt8(value);
                    break;
                case SoundIoFormatS16LE:
                    *reinterpret_cast<qint16 *>(p) = floatToInt16(value);
                    break;
                case SoundIoFormatS24LE:
                    *reinterpret_cast<qint32 *>(p) = floatToInt24(value);
                    break;
                case SoundIoFormatS32LE:
                    *reinterpret_cast<qint32 *>(p) = floatToInt32(value);
                    break;
                case SoundIoFormatFloat64LE:
                    *reinterpret_cast<double *>(p) = value;
                    break;
                default:
                    Q_UNREACHABLE();
            }
        }
        float *writePointerTo(int channel, qint64 startPos) override {
            if (isContinuous()) {
                return reinterpret_cast<float *>(m_area[channel].ptr + startPos * m_area[channel].step);
            }
            return nullptr;
        }
    };

    SoundIOAudioDevice::SoundIOAudioDevice(const QString &name, SoundIOAudioDriver *driver)
        : AudioDevice(*new SoundIOAudioDevicePrivate, driver) {
        Q_D(SoundIOAudioDevice);
        setDriver(driver);
        SoundIo *soundio = driver->d_func()->soundio;
        m_isVirtualDefault = name.isEmpty();
        if (m_isVirtualDefault) {
            m_actualDeviceName.clear();
            int defaultIdx = soundio_default_output_device_index(soundio);
            if (defaultIdx >= 0) {
                if (SoundIoDevice *defaultDev = soundio_get_output_device(soundio, defaultIdx)) {
                    m_actualDeviceName = QString::fromUtf8(defaultDev->name);
                    soundio_device_unref(defaultDev);
                }
            }
            connect(driver, &SoundIOAudioDriver::aboutToChangeDevice, this, &SoundIOAudioDevice::handleDeviceChanged);
            setName("");
        } else {
            m_actualDeviceName = name;
            setName(name);
        }

        int deviceCount = soundio_output_device_count(soundio);
        d->device = nullptr;
        
        for (int i = 0; i < deviceCount; i++) {
            SoundIoDevice *device = soundio_get_output_device(soundio, i);
            if (device && QString::fromUtf8(device->name) == m_actualDeviceName && device->is_raw == static_cast<SoundIOAudioDriver *>(d->driver.get())->d_func()->isRaw) {
                d->device = device;
                break;
            } else if (device) {
                soundio_device_unref(device);
            }
        }
        
        if (!d->device) {
            setErrorString("SoundIO: Device not found: " + m_actualDeviceName);
            return;
        }
        
        setChannelCount(d->device->current_layout.channel_count);
        setAvailableBufferSizes(COMMON_BUFFER_SIZES);
        setPreferredBufferSize(1024);
        
        QList<double> availableSampleRates;
        for (double rate : COMMON_SAMPLE_RATES) {
            if (soundio_device_supports_sample_rate(d->device, static_cast<int>(rate))) {
                availableSampleRates.append(rate);
            }
        }
        
        if (availableSampleRates.isEmpty()) {
            if (d->device->sample_rate_current > 0) {
                availableSampleRates.append(d->device->sample_rate_current);
            } else {
                availableSampleRates.append(48000);
            }
        }
        
        setAvailableSampleRates(availableSampleRates);
        
        if (d->device->sample_rate_current > 0) {
            setPreferredSampleRate(d->device->sample_rate_current);
        } else {
            setPreferredSampleRate(48000);
        }
        
        setIsInitialized(true);
    }

    SoundIOAudioDevice::~SoundIOAudioDevice() {
        SoundIOAudioDevice::close();
    }

    inline SoundIoFormat selectFormat(SoundIoDevice *device) {
        return *std::max_element(device->formats, device->formats + device->format_count, [](SoundIoFormat a, SoundIoFormat b) {
            if (a == SoundIoFormatFloat32LE)
                return false;
            if (b == SoundIoFormatFloat32LE)
                return true;
            return a < b;
        });
    }

    bool SoundIOAudioDevice::open(qint64 bufferSize, double sampleRate) {
        Q_D(SoundIOAudioDevice);
        
        if (!isInitialized() || !d->device)
            return false;
            
        if (d->isOpen)
            close();
            
        // Create output stream
        d->outStream = soundio_outstream_create(d->device);
        if (!d->outStream) {
            setErrorString("SoundIO: Failed to create output stream");
            return false;
        }

        // Configure the stream
        d->outStream->format = selectFormat(d->device);
        d->outStream->sample_rate = static_cast<int>(sampleRate);
        d->outStream->layout = d->device->current_layout;
        d->outStream->write_callback = SoundIOAudioDevicePrivate::writeCallback;
        d->outStream->underflow_callback = SoundIOAudioDevicePrivate::underflowCallback;
        d->outStream->error_callback = SoundIOAudioDevicePrivate::errorCallback;
        d->outStream->userdata = d;
        
        // Open the stream
        int err = soundio_outstream_open(d->outStream);
        if (err) {
            setErrorString(QString("SoundIO: Error opening device: %1").arg(soundio_strerror(err)));
            soundio_outstream_destroy(d->outStream);
            d->outStream = nullptr;
            return false;
        }
        
        d->isOpen = true;
        return AudioStreamBase::open(bufferSize, sampleRate);
    }

    void SoundIOAudioDevice::close() {
        Q_D(SoundIOAudioDevice);
        if (!d->driver || !static_cast<SoundIOAudioDriver *>(d->driver.get())->d_func()->soundio)
            return;
        
        stop();
        
        if (d->outStream) {
            soundio_outstream_destroy(d->outStream);
            d->outStream = nullptr;
        }
        
        if (d->device) {
            soundio_device_unref(d->device);
            d->device = nullptr;
        }
        
        d->isOpen = false;
        AudioDevice::close();
    }

    bool SoundIOAudioDevice::start(AudioDeviceCallback *audioDeviceCallback) {
        Q_D(SoundIOAudioDevice);
        
        if (!isOpen() || !d->outStream || isStarted())
            return false;
            
        d->audioDeviceCallback = audioDeviceCallback;
        
        int err = soundio_outstream_start(d->outStream);
        if (err) {
            setErrorString(QString("SoundIO: Error starting device: %1").arg(soundio_strerror(err)));
            d->audioDeviceCallback = nullptr;
            return false;
        }
        
        d->isStarted = true;
        return true;
    }

    void SoundIOAudioDevice::stop() {
        Q_D(SoundIOAudioDevice);
        
        if (!isStarted() || !d->outStream)
            return;
            
        soundio_outstream_pause(d->outStream, true);
        d->audioDeviceCallback = nullptr;
        d->isStarted = false;
    }

    void SoundIOAudioDevice::lock() {
        Q_D(SoundIOAudioDevice);
        d->mutex.lock();
    }

    void SoundIOAudioDevice::unlock() {
        Q_D(SoundIOAudioDevice);
        d->mutex.unlock();
    }

    bool SoundIOAudioDevice::openControlPanel() {
        return false;
    }
    void SoundIOAudioDevicePrivate::writeCallback(SoundIoOutStream *outStream, int frameCountMin, int frameCountMax) {
        auto *d = static_cast<SoundIOAudioDevicePrivate *>(outStream->userdata);
        if (!d->audioDeviceCallback)
            return;
        auto *device = qobject_cast<AudioDevice*>(d->q_func());
        qint64 deviceBufferSize = device->bufferSize();
        int channelCount = outStream->layout.channel_count;
        
        int frameCount = qBound(frameCountMin, static_cast<int>(deviceBufferSize), frameCountMax);
        
        SoundIoChannelArea *areas;
        int err = soundio_outstream_begin_write(outStream, &areas, &frameCount);
        if (err) {
            qWarning() << "SoundIO: Error writing to device:" << soundio_strerror(err);
            return;
        }
        
        if (!frameCount)
            return;
        
        QMutexLocker locker(&d->mutex);
        if (d->audioDeviceCallback) {
            if (frameCount <= deviceBufferSize) {
                SoundIOChannelAreaWrapper buffer(areas, outStream->format, channelCount, frameCount);
                buffer.clear();
                d->audioDeviceCallback->workCallback(&buffer);
            } else {
                qint64 framesProcessed = 0;
                while (framesProcessed < frameCount) {
                    qint64 framesToProcess = qMin(deviceBufferSize, frameCount - framesProcessed);
                    SoundIOChannelAreaWrapper buffer(areas, outStream->format, channelCount, framesToProcess);
                    buffer.clear();
                    d->audioDeviceCallback->workCallback(&buffer);
                    framesProcessed += framesToProcess;
                }
            }
        }
        
        err = soundio_outstream_end_write(outStream);
        if (err) {
            qWarning() << "SoundIO: Error ending write:" << soundio_strerror(err);
        }
    }

    void SoundIOAudioDevicePrivate::underflowCallback(SoundIoOutStream *outStream) {
        qWarning() << "SoundIO: Buffer underflow occurred";
    }

    void SoundIOAudioDevicePrivate::errorCallback(SoundIoOutStream *outStream, int err) {
        auto *d = static_cast<SoundIOAudioDevicePrivate *>(outStream->userdata);
        qWarning() << "SoundIO: Stream error:" << soundio_strerror(err);
        if (err == SoundIoErrorStreaming)
            QMetaObject::invokeMethod(d->q_func(), "handleStreamingError", Qt::QueuedConnection);
    }

    void SoundIOAudioDevice::handleStreamingError() {
        close();
        emit closed();
    }

    void SoundIOAudioDevice::handleDeviceChanged() {
        Q_D(SoundIOAudioDevice);
        if (!m_isVirtualDefault)
            return;
        auto *drv = static_cast<SoundIOAudioDriver *>(driver());
        SoundIo *soundio = drv->d_func()->soundio;
        QString newDefault;
        int defaultIdx = soundio_default_output_device_index(soundio);
        if (defaultIdx >= 0) {
            if (SoundIoDevice *defaultDev = soundio_get_output_device(soundio, defaultIdx)) {
                newDefault = QString::fromUtf8(defaultDev->name);
                soundio_device_unref(defaultDev);
            }
        }
        if (newDefault == m_actualDeviceName)
            return;
        // Save current state
        bool wasOpen = isOpen();
        bool wasStarted = isStarted();
        qint64 oldBufferSize = bufferSize();
        double oldSampleRate = sampleRate();
        AudioDeviceCallback *cb = nullptr;
        if (wasStarted) cb = d->audioDeviceCallback;
        // Close current device
        close();
        // Re-init with new default device
        m_actualDeviceName = newDefault;
        // Re-run device search and initialization logic, use m_actualDeviceName
        if (!soundio) {
            setErrorString("SoundIO: Driver not initialized");
            return;
        }
        int deviceCount = soundio_output_device_count(soundio);
        d->device = nullptr;
        for (int i = 0; i < deviceCount; i++) {
            SoundIoDevice *device = soundio_get_output_device(soundio, i);
            if (device && QString::fromUtf8(device->name) == m_actualDeviceName && device->is_raw == drv->d_func()->isRaw) {
                d->device = device;
                break;
            } else if (device) {
                soundio_device_unref(device);
            }
        }
        if (!d->device) {
            setErrorString("SoundIO: Device not found: " + m_actualDeviceName);
            return;
        }
        setChannelCount(d->device->current_layout.channel_count);
        setAvailableBufferSizes(COMMON_BUFFER_SIZES);
        setPreferredBufferSize(1024);
        QList<double> availableSampleRates;
        for (double rate : COMMON_SAMPLE_RATES) {
            if (soundio_device_supports_sample_rate(d->device, static_cast<int>(rate))) {
                availableSampleRates.append(rate);
            }
        }
        if (availableSampleRates.isEmpty()) {
            if (d->device->sample_rate_current > 0) {
                availableSampleRates.append(d->device->sample_rate_current);
            } else {
                availableSampleRates.append(48000);
            }
        }
        setAvailableSampleRates(availableSampleRates);
        if (d->device->sample_rate_current > 0) {
            setPreferredSampleRate(d->device->sample_rate_current);
        } else {
            setPreferredSampleRate(48000);
        }
        setIsInitialized(true);
        // Restore state
        if (wasOpen) open(oldBufferSize, oldSampleRate);
        if (wasStarted && cb) start(cb);
    }

}