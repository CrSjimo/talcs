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


    signals:
        void meterUpdated(float magnitudeLeft, float magnitudeRight);

    protected:
        explicit PositionableMixerAudioSource(PositionableMixerAudioSourcePrivate &d, QObject *parent);
    };

}

#endif // TALCS_POSITIONABLEMIXERAUDIOSOURCE_H
