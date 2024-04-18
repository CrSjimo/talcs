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

#ifndef AUDIODEVICE_H
#define AUDIODEVICE_H

#include <QObject>

#include <TalcsCore/AudioStreamBase.h>
#include <TalcsCore/ErrorStringProvider.h>
#include <TalcsCore/NameProvider.h>
#include <TalcsDevice/AudioDeviceCallback.h>

namespace talcs {
    class AudioDevicePrivate;
    class AudioDriver;

    class TALCSDEVICE_EXPORT AudioDevice : public QObject,
                                           public AudioStreamBase,
                                           public NameProvider,
                                           public ErrorStringProvider {
        Q_OBJECT
        Q_DECLARE_PRIVATE(AudioDevice)

    public:
        ~AudioDevice() override;

        bool isInitialized() const;

        AudioDriver *driver() const;

        int channelCount() const;

        int activeChannelCount() const;

        void setActiveChannelCount(int num);

        QList<qint64> availableBufferSizes() const;
        qint64 preferredBufferSize() const;
        QList<double> availableSampleRates() const;
        double preferredSampleRate() const;

        virtual bool start(AudioDeviceCallback *audioDeviceCallback);
        bool isStarted() const;
        virtual void stop();

        virtual void lock();
        virtual void unlock();

        virtual bool openControlPanel();

        void close() override;

    signals:
        void closed();

    protected:
        friend class AudioDriver;
        explicit AudioDevice(QObject *parent = nullptr);
        AudioDevice(AudioDevicePrivate &d, QObject *parent);
        QScopedPointer<AudioDevicePrivate> d_ptr;

        void setIsInitialized(bool isInitialized);

        void setDriver(AudioDriver *driver);

        void setChannelCount(int channelCount);

        void setAvailableBufferSizes(const QList<qint64> &bufferSizes);
        void setPreferredBufferSize(qint64 bufferSize);
        void setAvailableSampleRates(const QList<double> &sampleRates);
        void setPreferredSampleRate(double sampleRate);
    };

    class TALCSDEVICE_EXPORT AudioDeviceLocker {
    public:
        inline explicit AudioDeviceLocker(talcs::AudioDevice *audioDevice) : m_dev(audioDevice) {
            if (m_dev)
                m_dev->lock();
        }

        inline ~AudioDeviceLocker() {
            if (m_dev)
                m_dev->unlock();
        }

    private:
        Q_DISABLE_COPY(AudioDeviceLocker);
        AudioDevice *m_dev;
    };
}

#endif // AUDIODEVICE_H
