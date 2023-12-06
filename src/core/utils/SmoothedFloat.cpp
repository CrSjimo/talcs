/******************************************************************************
 * Copyright (c) 2023 CrSjimo                                                 *
 *                                                                            *
 * This file is part of TALCS.                                                *
 *                                                                            *
 * TALCS is free software: you can redistribute it and/or modify it under the *
 * terms of the GNU Lesser General Public License as published by the Free    *
 * Software Foundation, either version 3 of the License, or (at your option)  *
 * any later version.                                                         *
 *                                                                            *
 * TALCS is distributed in the hope that it will be useful, but WITHOUT ANY   *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS  *
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for    *
 * more details.                                                              *
 *                                                                            *
 * You should have received a copy of the GNU Lesser General Public License   *
 * along with TALCS. If not, see <https://www.gnu.org/licenses/>.             *
 ******************************************************************************/

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
    
}