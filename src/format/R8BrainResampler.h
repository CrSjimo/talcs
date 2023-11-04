#ifndef TALCS_R8BRAINRESAMPLER_H
#define TALCS_R8BRAINRESAMPLER_H

#include <functional>

#include <QScopedPointer>

#include <TalcsFormat/TalcsFormatGlobal.h>

namespace r8b {
    class CDSPResampler;
}

namespace talcs {

    class R8BrainResamplerPrivate;

    class TALCSFORMAT_EXPORT R8BrainResampler {
        friend class R8BrainResamplerPrivate;
        using CallbackFunction = std::function<void(float *, qint64)>;
    public:
        explicit R8BrainResampler(double ratio, qint64 bufferSize, const CallbackFunction &callback);
        ~R8BrainResampler();

        void reset();

        void process(float *buffer);

    private:
        QScopedPointer<R8BrainResamplerPrivate> d;

    };

} // talcs

#endif //TALCS_R8BRAINRESAMPLER_H
