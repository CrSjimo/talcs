#ifndef TALCS_MIXERAUDIOSOURCE_H
#define TALCS_MIXERAUDIOSOURCE_H

#include <QList>
#include <QObject>

#include "AudioSource.h"
#include "IMixer.h"

namespace talcs {
    class MixerAudioSourcePrivate;

    class TALCS_EXPORT MixerAudioSource : public QObject, public AudioSource, public IMixer<AudioSource> {
        Q_OBJECT
#define d_ptr AudioSource::d_ptr
        Q_DECLARE_PRIVATE(MixerAudioSource)
#undef d_ptr
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

        void setGain(float gain) override;
        float gain() const override;

        void setPan(float pan) override;
        float pan() const override;

        void setRouteChannels(bool routeChannels) override;
        bool routeChannels() const override;

    signals:
        void meterUpdated(float leftMagnitude, float rightMagnitude);

    protected:
        explicit MixerAudioSource(MixerAudioSourcePrivate &d, QObject *parent);
    };
}



#endif // TALCS_MIXERAUDIOSOURCE_H