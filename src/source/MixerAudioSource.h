#ifndef TALCS_MIXERAUDIOSOURCE_H
#define TALCS_MIXERAUDIOSOURCE_H

#include <QList>
#include <QObject>

#include "AudioSource.h"

namespace talcs {
    class MixerAudioSourcePrivate;

    class TALCS_EXPORT MixerAudioSource : public QObject, public AudioSource {
        Q_OBJECT
#define d_ptr AudioSource::d_ptr
        Q_DECLARE_PRIVATE(MixerAudioSource)
#undef d_ptr
    public:
        MixerAudioSource();
        ~MixerAudioSource();

        bool open(qint64 bufferSize, double sampleRate) override;
        qint64 read(const AudioSourceReadData &readData) override;
        void close() override;

        bool addSource(AudioSource *src, bool takeOwnership = false);
        bool removeSource(AudioSource *src);
        void removeAllSource();
        QList<AudioSource *> sources() const;

        void setGain(float gain);
        float gain() const;

        void setPan(float pan);
        float pan() const;

        void setRouteChannels(bool routeChannels);
        bool routeChannels() const;

    signals:
        void meterUpdated(float leftMagnitude, float rightMagnitude);

    protected:
        explicit MixerAudioSource(MixerAudioSourcePrivate &d);
    };
}



#endif // TALCS_MIXERAUDIOSOURCE_H
