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

    /**
     * @class SmoothedFloat
     * @brief A helper for smoothing float values.
     */

    /**
     * Constructor.
     */
    SmoothedFloat::SmoothedFloat() : d(new SmoothedFloatPrivate) {
    }

    SmoothedFloat::SmoothedFloat(const SmoothedFloat &o) = default;

    /**
     * Constructor.
     * @param initialValue the initial value
     */
    SmoothedFloat::SmoothedFloat(float initialValue) : d(new SmoothedFloatPrivate) {
        d->currentValue = d->targetValue = initialValue;
    }

    /**
     * Destructor.
     */
    SmoothedFloat::~SmoothedFloat() = default;

    SmoothedFloat &SmoothedFloat::operator=(const SmoothedFloat &o) = default;

    void SmoothedFloatPrivate::setStepSize() {
        stepSize = (targetValue - currentValue) / float(countdown);
    }

    /**
     * Sets how many steps does the smoothing procedure takes.
     */
    void SmoothedFloat::setRampLength(int steps) {
        d->steps = steps;
        setCurrentAndTargetValue(d->targetValue);
    }

    /**
     * Sets the target value of smoothing.
     */
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

    /**
     * Sets the target value of smoothing and make it the current value.
     */
    void SmoothedFloat::setCurrentAndTargetValue(float value) {
        d->currentValue = d->targetValue = value;
        d->countdown = 0;
    }

    /**
     * Gets the current value.
     */
    float SmoothedFloat::currentValue() const {
        return d->currentValue;
    }

    /**
     * Gets the target value.
     */
    float SmoothedFloat::targetValue() const {
        return d->targetValue;
    }

    /**
     * Gets the computed next smoothed value and update the internal state.
     * @param steps the number of steps
     */
    float SmoothedFloat::nextValue(int steps) {
        if (steps >= d->countdown) {
            setCurrentAndTargetValue(d->targetValue);
            return d->targetValue;
        }
        d->currentValue += d->stepSize * float(steps);
        d->countdown -= steps;
        return d->currentValue;
    }

    /**
     * Gets whether the internal state is smoothing.
     */
    bool SmoothedFloat::isSmoothing() const {
        return d->countdown > 0;
    }
    
}