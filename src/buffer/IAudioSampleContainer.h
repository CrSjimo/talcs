#ifndef TALCS_IAUDIOSAMPLECONTAINER_H
#define TALCS_IAUDIOSAMPLECONTAINER_H

#include "IAudioSampleProvider.h"

namespace talcs {
    class TALCS_EXPORT IAudioSampleContainer : public IAudioSampleProvider {
    public:
        virtual float &sampleAt(int channel, qint64 pos) = 0;
        virtual float *writePointerTo(int channel, qint64 startPos);

        void setSampleRange(int destChannel, qint64 destStartPos, qint64 length, const IAudioSampleProvider &src,
                            int srcChannel, qint64 srcStartPos);
        void setSampleRange(const IAudioSampleProvider &src);

        void addSampleRange(int destChannel, qint64 destStartPos, qint64 length, const IAudioSampleProvider &src,
                            int srcChannel, qint64 srcStartPos, float gain = 1);
        void addSampleRange(const IAudioSampleProvider &src, float gain = 1);

        void gainSampleRange(int destChannel, qint64 destStartPos, qint64 length, float gain);
        void gainSampleRange(int destChannel, float gain);
        void gainSampleRange(float gain);

        void clear(int destChannel, qint64 destStartPos, qint64 length);
        void clear(int destChannel);
        void clear();
    };
}



#endif // TALCS_IAUDIOSAMPLECONTAINER_H