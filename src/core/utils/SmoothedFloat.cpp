#include "SmoothedFloat.h"
#include "SmoothedFloat_p.h"

namespace talcs {
    SmoothedFloat::SmoothedFloat() : d(new SmoothedFloatPrivate) {
    }

    SmoothedFloat::SmoothedFloat(float initialValue) : d(new SmoothedFloatPrivate) {
        d->currentValue = d->targetValue = initialValue;
    }

    SmoothedFloat::~SmoothedFloat() = default;

    void SmoothedFloatPrivate::setStepSize() {
        stepSize = (targetValue - currentValue) / float(countdown);
    }
    void SmoothedFloat::setRampLength(int steps) {
        d->steps = steps;
        setCurrentAndTargetValue(d->targetValue);
    }
    void SmoothedFloat::setTargetValue(float value) {
        if (qFuzzyCompare(value, d->targetValue))
            return;

        if (d->steps <= 0) {
            setCurrentAndTargetValue(value);
            return;
        }

        d->targetValue = value;
        d->countdown = d->steps;
        d->setStepSize();
    }
    void SmoothedFloat::setCurrentAndTargetValue(float value) {
        d->currentValue = d->targetValue = value;
        d->countdown = 0;
    }
    float SmoothedFloat::currentValue() const {
        return d->currentValue;
    }
    float SmoothedFloat::targetValue() const {
        return d->targetValue;
    }
    float SmoothedFloat::nextValue(int steps) {
        if (steps >= d->countdown) {
            setCurrentAndTargetValue(d->targetValue);
            return d->targetValue;
        }
        d->currentValue += d->stepSize * float(steps);
        d->countdown -= steps;
        return d->currentValue;
    }
    bool SmoothedFloat::isSmoothing() const {
        return d->countdown > 0;
    }
} // talcs