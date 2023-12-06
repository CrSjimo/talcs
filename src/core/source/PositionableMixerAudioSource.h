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

#ifndef TALCS_POSITIONABLEMIXERAUDIOSOURCE_H
#define TALCS_POSITIONABLEMIXERAUDIOSOURCE_H

#include <QMutex>
#include <QObject>

#include <TalcsCore/IMixer.h>
#include <TalcsCore/PositionableAudioSource.h>

namespace talcs {

    class PositionableMixerAudioSourcePrivate;

    class TALCSCORE_EXPORT PositionableMixerAudioSource : public QObject, public PositionableAudioSource, public IMixer<PositionableAudioSource> {
        Q_OBJECT
        Q_DECLARE_PRIVATE_D(PositionableAudioSource::d_ptr, PositionableMixerAudioSource)
    public:
        explicit PositionableMixerAudioSource(QObject *parent = nullptr);
        ~PositionableMixerAudioSource() override;
        bool open(qint64 bufferSize, double sampleRate) override;
        qint64 read(const AudioSourceReadData &readData) override;
        void close() override;
        qint64 length() const override;
        void setNextReadPosition(qint64 pos) override;

        bool addSource(PositionableAudioSource *src, bool takeOwnership = false) override;
        bool removeSource(PositionableAudioSource *src) override;
        void removeAllSources() override;
        QList<PositionableAudioSource *> sources() const override;
        void setSourceSolo(PositionableAudioSource *src, bool isSolo) override;
        bool isSourceSolo(PositionableAudioSource *src) const override;

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
        void meterUpdated(float magnitudeLeft, float magnitudeRight);

    protected:
        explicit PositionableMixerAudioSource(PositionableMixerAudioSourcePrivate &d, QObject *parent);
    };

}

#endif // TALCS_POSITIONABLEMIXERAUDIOSOURCE_H
