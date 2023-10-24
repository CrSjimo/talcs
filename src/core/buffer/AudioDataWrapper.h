#ifndef TALCS_AUDIODATAWRAPPER_H
#define TALCS_AUDIODATAWRAPPER_H

#include <QScopedPointer>

#include <TalcsCore/IAudioSampleContainer.h>

namespace talcs {

    class AudioDataWrapperPrivate;

    class TALCSCORE_EXPORT AudioDataWrapper : public IAudioSampleContainer {
        Q_DECLARE_PRIVATE(AudioDataWrapper)
    public:
        AudioDataWrapper(float *const *data, int channelCount, qint64 sampleCount, qint64 startPos = 0);
        ~AudioDataWrapper() override;

        float &sampleAt(int channel, qint64 pos) override;
        float constSampleAt(int channel, qint64 pos) const override;

        int channelCount() const override;
        qint64 sampleCount() const override;

        float *writePointerTo(int channel, qint64 startPos) override;
        const float *readPointerTo(int channel, qint64 startPos) const override;

        bool isContinuous() const override;

        float *data(int channel) const;

        void reset(float *const *data, int channelCount, qint64 sampleCount, qint64 startPos = 0);

    protected:
        QScopedPointer<AudioDataWrapperPrivate> d_ptr;
        AudioDataWrapper(AudioDataWrapperPrivate &d);
    };
    
}

#endif // TALCS_AUDIODATAWRAPPER_H
