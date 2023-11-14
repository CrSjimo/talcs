#ifndef TALCS_AUDIOFORMATINPUTSOURCE_P_H
#define TALCS_AUDIOFORMATINPUTSOURCE_P_H

#include <TalcsCore/private/PositionableAudioSource_p.h>
#include <TalcsFormat/AudioFormatInputSource.h>

#include "R8BrainMultichannelResampler.h"

#include <QMutex>
#include <QVector>

namespace talcs {
    class AudioFormatIO;

    class AudioFormatInputSourcePrivate : public PositionableAudioSourcePrivate {
        Q_DECLARE_PUBLIC(AudioFormatInputSource);
    public:
        AudioFormatIO *io = nullptr;
        bool takeOwnership;
        double ratio = 0;

        class AudioFormatInputResampler : public R8BrainMultichannelResampler {
        public:
            AudioFormatInputResampler(double ratio, qint64 bufferSize, int channelCount, AudioFormatInputSourcePrivate *d);
            void read(const talcs::AudioSourceReadData &readData) override;
            AudioFormatInputSourcePrivate *d;
            QVector<float> tmpBuf;
        };

        AudioFormatInputResampler *resampler = nullptr;

        qint64 inPosition = 0;

        bool doStereoize = true;

        QMutex mutex;

    };
}

#endif // TALCS_AUDIOFORMATINPUTSOURCE_P_H
