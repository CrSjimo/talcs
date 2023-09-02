#ifndef TALCS_INTERLEAVEDAUDIODATAWRAPPER_H
#define TALCS_INTERLEAVEDAUDIODATAWRAPPER_H

#include <QScopedPointer>

#include "IAudioSampleContainer.h"

namespace talcs {
    class InterleavedAudioDataWrapperPrivate;

    class TALCS_EXPORT InterleavedAudioDataWrapper : public IAudioSampleContainer {
        Q_DECLARE_PRIVATE(InterleavedAudioDataWrapper)
    public:
        InterleavedAudioDataWrapper(float *data, int channelCount, qint64 sampleCount);
        ~InterleavedAudioDataWrapper();

        float &sampleAt(int channel, qint64 pos) override;
        float constSampleAt(int channel, qint64 pos) const override;
        int channelCount() const override;
        qint64 sampleCount() const override;

        float *writePointerTo(int channel, qint64 startPos) override;
        const float *readPointerTo(int channel, qint64 startPos) const override;

        bool isContinuous() const override;

        float *data() const;
        void reset(float *data, int channelCount, qint64 sampleCount);

    protected:
        QScopedPointer<InterleavedAudioDataWrapperPrivate> d_ptr;
        InterleavedAudioDataWrapper(InterleavedAudioDataWrapperPrivate &d);
    };
}

#endif // TALCS_INTERLEAVEDAUDIODATAWRAPPER_H
