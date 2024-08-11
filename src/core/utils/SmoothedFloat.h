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

#ifndef TALCS_SMOOTHEDFLOAT_H
#define TALCS_SMOOTHEDFLOAT_H

#include <QSharedDataPointer>

#include <TalcsCore/TalcsCoreGlobal.h>

namespace talcs {

    class SmoothedFloatPrivate;

    class TALCSCORE_EXPORT SmoothedFloat {
    public:
        SmoothedFloat();
        SmoothedFloat(const SmoothedFloat &o);
        SmoothedFloat(float initialValue);
        ~SmoothedFloat();

        SmoothedFloat &operator=(const SmoothedFloat &o);

        void setRampLength(int steps);

        void setTargetValue(float value);
        void setCurrentAndTargetValue(float value);
        inline SmoothedFloat &operator=(float value) {
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
