#ifndef TALCS_DECIBELS_H
#define TALCS_DECIBELS_H

#include <QString>

#include "global/TalcsGlobal.h"

namespace talcs {

    class TALCS_EXPORT Decibels {
    public:
        static float decibelsToGain(float decibels, float minusInfinityDb = -96);
        static float gainToDecibels(float gain, float minusInfinityDb = -96);
        static QString toString(float decibels, int precision = 1, float minusInfinityDb = -96);
    };

} // talcs

#endif // TALCS_DECIBELS_H
