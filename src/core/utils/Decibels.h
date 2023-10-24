#ifndef TALCS_DECIBELS_H
#define TALCS_DECIBELS_H

#include <QString>

#include <TalcsCore/TalcsCoreGlobal.h>

namespace talcs {

    class TALCSCORE_EXPORT Decibels {
    public:
        static float decibelsToGain(float decibels, float minusInfinityDb = -96);
        static float gainToDecibels(float gain, float minusInfinityDb = -96);
        static QString toString(float decibels, int precision = 1, float minusInfinityDb = -96);
    };

}

#endif // TALCS_DECIBELS_H
