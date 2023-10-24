#ifndef TALCS_AUDIOBUFFER_H
#define TALCS_AUDIOBUFFER_H

#include <QVector>

#include "IAudioSampleContainer.h"

namespace talcs {
    class TALCS_EXPORT AudioBuffer : public IAudioSampleContainer {
    public:
        AudioBuffer();
        AudioBuffer(int channelCount, qint64 sampleCount);

        float &sampleAt(int channel, qint64 pos) override;
        float constSampleAt(int channel, qint64 pos) const override;

        float *writePointerTo(int channel, qint64 startPos) override;
        const float *readPointerTo(int channel, qint64 startPos) const override;

        int channelCount() const override;
        qint64 sampleCount() const override;

        bool isContinuous() const override;

        void resize(int newChannelCount = -1, qint64 newSampleCount = -1);

        float *data(int channel);
        float const *constData(int channel) const;
        QVector<float> &vector(int channel);
        const QVector<float> &constVector(int channel) const;

        AudioBuffer slice(int startChannelIndex, qint64 startSampleCount, int channelSize = -1,
                          qint64 length = -1) const;

        static AudioBuffer from(const IAudioSampleProvider &src);

    private:
        QVector<QVector<float>> m_buffer;
    };
}

#endif // TALCS_AUDIOBUFFER_H
