#ifndef TALCS_IAUDIOSAMPLEPROVIDER_H
#define TALCS_IAUDIOSAMPLEPROVIDER_H

#include <TalcsCore/TalcsCoreGlobal.h>

namespace talcs {

    class TALCSCORE_EXPORT IAudioSampleProvider {
    public:
        virtual ~IAudioSampleProvider() = default;

        virtual float constSampleAt(int channel, qint64 pos) const = 0;
        virtual bool isContinuous() const;
        virtual const float *readPointerTo(int channel, qint64 startPos) const;

        virtual int channelCount() const = 0;
        virtual qint64 sampleCount() const = 0;

        float magnitude(int channel, qint64 startPos, qint64 length) const;
        float magnitude(int channel) const;

        float rms(int channel, qint64 startPos, qint64 length) const;
        float rms(int channel) const;
    };

}

#endif // TALCS_IAUDIOSAMPLEPROVIDER_H
