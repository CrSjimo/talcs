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
        SourceIterator appendSource(talcs::AudioSource *src, bool takeOwnership) override;
        SourceIterator prependSource(talcs::AudioSource *src, bool takeOwnership) override;
        SourceIterator insertSource(const SourceIterator &pos, talcs::AudioSource *src, bool takeOwnership) override;

        bool removeSource(AudioSource *src) override;
        void eraseSource(const SourceIterator &srcIt) override;
        void removeAllSources() override;

        void moveSource(const SourceIterator &pos, const SourceIterator &target) override;
        void swapSource(const SourceIterator &first, const SourceIterator &second) override;

        QList<AudioSource *> sources() const override;
        SourceIterator firstSource() const override;
        SourceIterator lastSource() const override;

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

        void setLevelMeterChannelCount(int count) override;
        int levelMeterChannelCount() override;

    signals:
        int levelMetered(const QVector<float> &values);

    protected:
        explicit MixerAudioSource(MixerAudioSourcePrivate &d, QObject *parent);
    };
}



#endif // TALCS_MIXERAUDIOSOURCE_H
