#ifndef TALCS_SMOOTHEDFLOAT_H
#define TALCS_SMOOTHEDFLOAT_H

#include <QSharedDataPointer>

#include <TalcsCore/TalcsCoreGlobal.h>

namespace talcs {

    class SmoothedFloatPrivate;

    class TALCSCORE_EXPORT SmoothedFloat {
    public:
        SmoothedFloat();
        SmoothedFloat(float initialValue);
        ~SmoothedFloat();

        void setRampLength(int steps);

        void setTargetValue(float value);
        void setCurrentAndTargetValue(float value);
        SmoothedFloat &operator=(float value) {
            setTargetValue(value);
            return *this;
        }

        float currentValue() const;
        float targetValue() const;
        float nextValue(int steps = 1);

        bool isSmoothing() const;

    private:
        QSharedDataPointer<SmoothedFloatPrivate> d;
    };

}

#endif // TALCS_SMOOTHEDFLOAT_H
