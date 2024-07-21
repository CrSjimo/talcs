/******************************************************************************
 * Copyright (c) 2024 CrSjimo                                                 *
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

#ifndef TALCS_ABSTRACTOUTPUTCONTEXT_H
#define TALCS_ABSTRACTOUTPUTCONTEXT_H

#include <QObject>

#include <TalcsDevice/TalcsDeviceGlobal.h>

namespace talcs {

    class AudioDevice;
    class AudioSourcePlayback;

    class MixerAudioSource;

    class AbstractOutputContextPrivate;

    class TALCSDEVICE_EXPORT AbstractOutputContext : public QObject {
        Q_OBJECT
        Q_DECLARE_PRIVATE(AbstractOutputContext)
    public:
        ~AbstractOutputContext() override;

        AudioDevice *device() const;
        AudioSourcePlayback *playback() const;
        MixerAudioSource *controlMixer() const;
        MixerAudioSource *preMixer() const;

    signals:
        void bufferSizeChanged(qint64 bufferSize);
        void sampleRateChanged(double sampleRate);
        void deviceChanged();

    protected:
        explicit AbstractOutputContext(QObject *parent = nullptr);

        void setDevice(AudioDevice *device);

    private:
        QScopedPointer<AbstractOutputContextPrivate> d_ptr;

    };

} // talcs

#endif //TALCS_ABSTRACTOUTPUTCONTEXT_H
