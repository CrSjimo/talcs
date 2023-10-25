#include "Decibels.h"

#include <cmath>

namespace talcs {

    float Decibels::decibelsToGain(float decibels, float minusInfinityDb) {
        return decibels > minusInfinityDb ? std::pow (10.0f, decibels * .05f) : .0f;
    }
    float Decibels::gainToDecibels(float gain, float minusInfinityDb) {
        return gain > .0f ? std::max (minusInfinityDb, std::log10 (gain) * 20.0f) : minusInfinityDb;
    }
    QString Decibels::toString(float decibels, int precision, float minusInfinityDb) {
        if (decibels <= minusInfinityDb)
            return "-INF dB";
        QString s;
        if (decibels >= .0f)
            s += "+";
        if (precision <= 0)
            s += QString::number(int(decibels));
        else
            s += QString::number(decibels, 'f', precision);
        s += " dB";
        return s;
    }
    
}