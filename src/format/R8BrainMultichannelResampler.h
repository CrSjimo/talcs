#ifndef TALCS_R8BRAINMULTICHANNELRESAMPLER_H
#define TALCS_R8BRAINMULTICHANNELRESAMPLER_H

#include <QScopedPointer>

#include <TalcsFormat/TalcsFormatGlobal.h>
#include <TalcsCore/AudioSource.h>

namespace talcs {

    class AudioBuffer;
    class AudioSourceReadData;
    class IAudioSampleProvider;

    class R8BrainMultichannelResamplerPrivate;
    class ChannelResampler;

    class TALCSFORMAT_EXPORT R8BrainMultichannelResampler {
    public:
        explicit R8BrainMultichannelResampler(double ratio, qint64 bufferSize, int channelCount);
        ~R8BrainMultichannelResampler();

        void reset();

        void process(const AudioSourceReadData &readData);

        double ratio() const;
        qint64 bufferSize() const;
        int channelCount() const;

    protected:
        virtual void read(const AudioSourceReadData &readData) = 0;

    private:
        friend class ChannelResampler;
        QScopedPointer<R8BrainMultichannelResamplerPrivate> d;

    };

} // talcs

#endif //TALCS_R8BRAINMULTICHANNELRESAMPLER_H
