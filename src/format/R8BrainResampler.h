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
    public:
        explicit R8BrainResampler(double ratio, qint64 bufferSize);
        ~R8BrainResampler();

        void reset();

        void process(float *buffer);

        double ratio() const;
        qint64 bufferSize() const;

    protected:
        virtual void read(float *inputBlock, qint64 length) = 0;

    private:
        QScopedPointer<R8BrainResamplerPrivate> d;

    };

} // talcs

#endif //TALCS_R8BRAINRESAMPLER_H
