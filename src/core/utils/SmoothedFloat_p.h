#ifndef TALCS_SMOOTHEDFLOAT_P_H
#define TALCS_SMOOTHEDFLOAT_P_H

#include <QSharedData>

#include <TalcsCore/SmoothedFloat.h>

namespace talcs {
    class SmoothedFloatPrivate : public QSharedData {
    public:
        float currentValue = 0;
        float targetValue = 0;

        int steps = 0;
        float stepSize = 0;
        int countdown = 0;

        void setStepSize();
    };
}

#endif // TALCS_SMOOTHEDFLOAT_P_H
