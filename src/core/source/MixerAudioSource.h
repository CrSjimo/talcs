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

#ifndef TALCS_MIXERAUDIOSOURCE_H
#define TALCS_MIXERAUDIOSOURCE_H

#include <QList>
#include <QObject>

#include <TalcsCore/AudioSource.h>
#include <TalcsCore/IMixer.h>

namespace talcs {
    class MixerAudioSourcePrivate;

    class TALCSCORE_EXPORT MixerAudioSource : public QObject, public AudioSource, public IMixer<AudioSource> {
        Q_OBJECT
        Q_DECLARE_PRIVATE_D(AudioSource::d_ptr, MixerAudioSource)
    public:
        explicit MixerAudioSource(QObject *parent = nullptr);
        ~MixerAudioSource() override;

        bool open(qint64 bufferSize, double sampleRate) override;
        qint64 read(const AudioSourceReadData &readData) override;
        void close() override;

        bool addSource(AudioSource *src, bool takeOwnership = false) override;
        bool removeSource(AudioSource *src) override;
        void removeAllSources() override;
        QList<AudioSource *> sources() const override;
        void setSourceSolo(AudioSource *src, bool isSolo) override;
        bool isSourceSolo(AudioSource *src) const override;

        void setGain(float gain) override;
        float gain() const override;

        void setPan(float pan) override;
        float pan() const override;

        void setRouteChannels(bool routeChannels) override;
        bool routeChannels() const override;

        void setSilentFlags(int silentFlags) override;
        int silentFlags() const override;

        void setMeterEnabled(bool enabled) override;
        bool isMeterEnabled() const override;

    signals:
        void meterUpdated(float leftMagnitude, float rightMagnitude);

    protected:
        explicit MixerAudioSource(MixerAudioSourcePrivate &d, QObject *parent);
    };
}



#endif // TALCS_MIXERAUDIOSOURCE_H
