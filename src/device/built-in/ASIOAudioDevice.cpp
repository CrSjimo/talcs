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

#include "ASIOAudioDevice.h"
#include "ASIOAudioDevice_p.h"

#include <QDebug>

#include "ASIOAudioDriver.h"
#include <TalcsCore/AudioBuffer.h>
#include <TalcsCore/AudioSampleConverter.h>
#include <TalcsCore/AudioSource.h>

namespace talcs {

    static const size_t DEVICE_LIST_SIZE = TALCS_ASIO_DEVICE_LIST_SIZE;

    static ASIOAudioDevicePrivate *m_devices[DEVICE_LIST_SIZE] = {};

    static void convertBuffer(void *dest, const float *src, qint64 length, ASIOSampleType type) {
        switch (type) {
            case ASIOSTInt16LSB:
                AudioSampleConverter::convertToInt16(dest, src, length, true);
                break;
            case ASIOSTInt16MSB:
                AudioSampleConverter::convertToInt16(dest, src, length, false);
                break;
            case ASIOSTInt24LSB:
                AudioSampleConverter::convertToInt24(dest, src, length, true);
                break;
            case ASIOSTInt24MSB:
                AudioSampleConverter::convertToInt24(dest, src, length, false);
                break;
            case ASIOSTInt32LSB:
                AudioSampleConverter::convertToInt32(dest, src, length, true);
                break;
            case ASIOSTInt32MSB:
                AudioSampleConverter::convertToInt32(dest, src, length, false);
                break;
            case ASIOSTFloat32LSB:
                AudioSampleConverter::convertToFloat32(dest, src, length, true);
                break;
            case ASIOSTFloat32MSB:
                AudioSampleConverter::convertToFloat32(dest, src, length, false);
                break;
            case ASIOSTFloat64LSB:
                AudioSampleConverter::convertToFloat64(dest, src, length, true);
                break;
            case ASIOSTFloat64MSB:
                AudioSampleConverter::convertToFloat64(dest, src, length, false);
                break;
            default:
                // unsupported sample type
                Q_ASSERT(false);
        }
    }

    template<int deviceIndex>
    struct CallbackFunctions {
        static void sampleRateDidChange(ASIOSampleRate sRate) {
            // TODO work on this
        }

        static long asioMessage(long selector, long value, void *message, double *opt) {
            // TODO work out on this
            // currently the parameters "value", "message" and "opt" are not used.
            long ret = 0;
            switch (selector) {
                case kAsioSelectorSupported:
                    if (value == kAsioResetRequest || value == kAsioEngineVersion || value == kAsioResyncRequest ||
                        value == kAsioLatenciesChanged
                        // the following three were added for ASIO 2.0, you don't necessarily have to support them
                        || value == kAsioSupportsTimeInfo || value == kAsioSupportsTimeCode ||
                        value == kAsioSupportsInputMonitor)
                        ret = 1L;
                    break;
                case kAsioResetRequest:
                    // defer the task and perform the reset of the driver during the next "safe" situation
                    // You cannot reset the driver right now, as this code is called from the driver.
                    // Reset the driver is done by completely destruct is. I.e. ASIOStop(), ASIODisposeBuffers(),
                    // Destruction Afterwards you initialize the driver again.
                    qDebug() << "kAsioResetRequest";
                    ret = 1L;
                    break;
                case kAsioResyncRequest:
                    // This informs the application, that the driver encountered some non fatal data loss.
                    // It is used for synchronization purposes of different media.
                    // Added mainly to work around the Win16Mutex problems in Windows 95/98 with the
                    // Windows Multimedia system, which could loose data because the Mutex was hold too long
                    // by another thread.
                    // However a driver can issue it in other situations, too.
                    qDebug() << "kAsioResyncRequest";
                    ret = 1L;
                    break;
                case kAsioLatenciesChanged:
                    // This will inform the host application that the drivers were latencies changed.
                    // Beware, it this does not mean that the buffer sizes have changed!
                    // You might need to update internal delay data.
                    ret = 1L;
                    break;
                case kAsioEngineVersion:
                    // return the supported ASIO version of the host application
                    // If a host applications does not implement this selector, ASIO 1.0 is assumed
                    // by the driver
                    ret = 2L;
                    break;
                case kAsioSupportsTimeInfo:
                    // informs the driver wether the asioCallbacks.bufferSwitchTimeInfo() callback
                    // is supported.
                    // For compatibility with ASIO 1.0 drivers the host application should always support
                    // the "old" bufferSwitch method, too.
                    ret = 1;
                    break;
                case kAsioSupportsTimeCode:
                    // informs the driver wether application is interested in time code info.
                    // If an application does not need to know about time code, the driver has less work
                    // to do.
                    ret = 0;
                    break;
            }
            return ret;
        }

        static ASIOTime *bufferSwitchTimeInfo(ASIOTime *timeInfo, long index, ASIOBool processNow) {
            auto d = m_devices[deviceIndex];
            QMutexLocker locker(&d->mutex);
            if (d->audioDeviceCallback) {
                d->audioDeviceCallback->workCallback(&d->audioBuffer);
            }
            for (int i = 0; i < d->audioBuffer.channelCount(); i++) {
                convertBuffer(d->bufferInfoList[i].buffers[index], d->audioBuffer.constData(i),
                              d->audioBuffer.sampleCount(), d->channelInfoList[i].type);
            }
            d->audioBuffer.clear();
            if (d->postOutput) {
                d->iasio->outputReady();
            }
            return nullptr;
        }

        static void bufferSwitch(long index, ASIOBool processNow) {
            ASIOTime t = {};
            bufferSwitchTimeInfo(&t, index, processNow);
        }
    };

    template<int maxDeviceIndex>
    constexpr ASIOCallbacks getASIOCallbacks(int deviceIndex) {
        if (deviceIndex == maxDeviceIndex) {
            return ASIOCallbacks{
                &CallbackFunctions<maxDeviceIndex>::bufferSwitch,
                &CallbackFunctions<maxDeviceIndex>::sampleRateDidChange,
                &CallbackFunctions<maxDeviceIndex>::asioMessage,
                &CallbackFunctions<maxDeviceIndex>::bufferSwitchTimeInfo,
            };
        } else {
            return getASIOCallbacks<maxDeviceIndex - 1>(deviceIndex);
        }
    }

    template<>
    constexpr ASIOCallbacks getASIOCallbacks<-1>(int deviceIndex) {
        return {};
    }

    void ASIOAudioDevicePrivate::setIASIOError() {
        Q_Q(ASIOAudioDevice);
        iasio->getErrorMessage(errorMessageBuffer);
        q->setErrorString(QString::fromLocal8Bit(errorMessageBuffer));
    }


    static const QList<double> COMMON_SAMPLE_RATES = {8000,   11025,  12000,  16000,  22050,  24000,
                                                      32000,  44100,  48000,  64000,  88200,  96000,
                                                      128000, 176400, 192000, 256000, 352800, 384000};

    /**
     * @class ASIOAudioDevice
     * @brief The audio device using ASIO
     * @see @link URL https://forums.steinberg .net/c/developer/asio/ @endlink
     */

    /**
     * Only ASIOAudioDevice can access this constructor.
     */
    ASIOAudioDevice::ASIOAudioDevice(const QString &name, IASIO *iasio, ASIOAudioDriver *driver)
        : AudioDevice(*new ASIOAudioDevicePrivate, driver) {
        Q_D(ASIOAudioDevice);
        auto *pDev = std::find(std::begin(m_devices), std::end(m_devices), nullptr);
        if (pDev == std::end(m_devices)) {
            qWarning() << "ASIOAudioDevice: Too many instances.";
            setErrorString("ASIOAudioDevice: Too many instances.");
            return;
        }
        *pDev = d;
        d->deviceIndex = int(std::distance(std::begin(m_devices), pDev));
        d->callbacks = getASIOCallbacks<DEVICE_LIST_SIZE - 1>(d->deviceIndex);
        setName(name);
        setDriver(driver);
        d->iasio = iasio;
        if (!d->iasio->init(nullptr)) {
            qWarning() << "ASIOAudioDevice: Initialization failed" << name;
            d->setIASIOError();
            return;
        }

        long numInputChannels, numOutputChannels;
        if (d->iasio->getChannels(&numInputChannels, &numOutputChannels) != ASE_OK) {
            d->setIASIOError();
            return;
        }
        setChannelCount(numOutputChannels);

        long minSize, preferredSize, maxSize, granularity;
        if (d->iasio->getBufferSize(&minSize, &maxSize, &preferredSize, &granularity) != ASE_OK) {
            d->setIASIOError();
            return;
        }
        setPreferredBufferSize(preferredSize);
        QList<qint64> bufferSizeList;
        if (granularity <= 0 || minSize > maxSize) {
            bufferSizeList.append(preferredSize);
        } else {
            for (long s = minSize; s <= maxSize; s += granularity) {
                bufferSizeList.append(s);
            }
        }
        setAvailableBufferSizes(bufferSizeList);

        double currentSampleRate;
        if (d->iasio->getSampleRate(&currentSampleRate) != ASE_OK) {
            d->setIASIOError();
            return;
        }
        setPreferredSampleRate(currentSampleRate);
        QList<double> sampleRateList;
        for (auto sr : COMMON_SAMPLE_RATES) {
            if (d->iasio->canSampleRate(sr) == ASE_OK) {
                sampleRateList.append(sr);
            }
        }
        setAvailableSampleRates(sampleRateList);

        d->postOutput = (d->iasio->outputReady() == ASE_OK);

        d->isInitialized = true;
        setErrorString({});
    }

    /**
     * Destructor.
     *
     * If the device is not close, it will be closed now.
     */
    ASIOAudioDevice::~ASIOAudioDevice() {
        Q_D(ASIOAudioDevice);
        ASIOAudioDevice::close();
        d->iasio->Release();
        if (d->deviceIndex != -1)
            m_devices[d->deviceIndex] = nullptr;
    }

    bool ASIOAudioDevice::open(qint64 bufferSize, double sampleRate) {
        if (!isInitialized())
            return false;
        Q_D(ASIOAudioDevice);
        ASIOAudioDevice::stop();
        if (d->iasio->setSampleRate(sampleRate) != ASE_OK) {
            d->setIASIOError();
            return false;
        }
        for (int i = 0; i < activeChannelCount(); i++) {
            d->bufferInfoList.append({
                ASIOFalse,
                i,
                {nullptr, nullptr},
            });
            ASIOChannelInfo channelInfo = {i, ASIOFalse};
            if (d->iasio->getChannelInfo(&channelInfo) != ASE_OK) {
                d->setIASIOError();
                return false;
            }
            d->channelInfoList.append(channelInfo);
        }
        if (d->iasio->createBuffers(d->bufferInfoList.data(), activeChannelCount(), bufferSize, &d->callbacks) != ASE_OK) {
            d->setIASIOError();
            return false;
        }
        setErrorString({});
        return AudioDevice::open(bufferSize, sampleRate);
    }
    void ASIOAudioDevice::close() {
        if (!isInitialized())
            return;
        Q_D(ASIOAudioDevice);
        ASIOAudioDevice::stop();
        d->iasio->disposeBuffers();
        d->bufferInfoList.clear();
        d->channelInfoList.clear();
        AudioDevice::close();
    }

    bool ASIOAudioDevice::start(AudioDeviceCallback * audioDeviceCallback) {
        if (!isInitialized())
            return false;
        Q_D(ASIOAudioDevice);
        QMutexLocker locker(&d->mutex);
        d->audioDeviceCallback = audioDeviceCallback;
        d->audioBuffer.resize(activeChannelCount(), bufferSize());
        if (!d->audioDeviceCallback->deviceWillStartCallback(this))
            return false;
        if (d->iasio->start() != ASE_OK) {
            d->setIASIOError();
            return false;
        }
        setErrorString({});
        return AudioDevice::start(audioDeviceCallback);
    }
    void ASIOAudioDevice::stop() {
        if (!isInitialized())
            return;
        Q_D(ASIOAudioDevice);
        QMutexLocker locker(&d->mutex);
        d->iasio->stop();
        if (d->audioDeviceCallback)
            d->audioDeviceCallback->deviceStoppedCallback();
        d->audioBuffer.resize(0, 0);
        d->audioDeviceCallback = nullptr;
        AudioDevice::stop();
    }

    void ASIOAudioDevice::lock() {
        Q_D(ASIOAudioDevice);
        d->mutex.lock();
    }
    void ASIOAudioDevice::unlock() {
        Q_D(ASIOAudioDevice);
        d->mutex.unlock();
    }

    bool ASIOAudioDevice::openControlPanel() {
        Q_D(ASIOAudioDevice);
        return d->iasio->controlPanel() == ASE_OK;
    }
}