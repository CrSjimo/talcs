//
// Created by Crs_1 on 2023/7/16.
//

#ifndef CHANNELROUTINGAUDIOSOURCE_H
#define CHANNELROUTINGAUDIOSOURCE_H

#include "AudioSource.h"

class ChannelRoutingAudioSourcePrivate;

class TALCS_EXPORT ChannelRoutingAudioSource: public AudioSource {
    Q_DECLARE_PRIVATE(ChannelRoutingAudioSource);
public:
    ChannelRoutingAudioSource();
    ~ChannelRoutingAudioSource();

    bool open(qint64 bufferSize, double sampleRate) override;
    qint64 read(const AudioSourceReadData &readData) override;
    void close() override;

    bool addSource(AudioSource *src, bool takeOwnership = false);
    bool removeSource(AudioSource *src);
    void removeAllSource();
    QList<AudioSource *> sources() const;

protected:
    ChannelRoutingAudioSource(ChannelRoutingAudioSourcePrivate &d);
};



#endif // CHANNELROUTINGAUDIOSOURCE_H
