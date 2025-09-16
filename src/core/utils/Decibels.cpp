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

#include "Decibels.h"

#include <cmath>

namespace talcs {

    /**
     * @class Decibels
     * @brief Helpful functions for decibel conversion.
     */

    /**
     * Converts decibel to gain.
     */
    float Decibels::decibelsToGain(double decibels, double minusInfinityDb) {
        return static_cast<float>(decibels > minusInfinityDb ? std::pow (10.0, decibels * .05) : .0);
    }

    /**
     * Converts gain to decibel.
     */
    double Decibels::gainToDecibels(float gain, double minusInfinityDb) {
        return gain > .0f ? qMax (minusInfinityDb, std::log10 (gain) * 20.0) : minusInfinityDb;
    }

    /**
     * Converts decibel to string.
     */
    QString Decibels::toString(double decibels, int precision, double minusInfinityDb) {
        if (decibels <= minusInfinityDb)
            return QStringLiteral("-INF dB");
        QString s;
        if (decibels >= .0)
            s += '+';
        if (precision <= 0)
            s += QString::number(static_cast<int>(decibels));
        else
            s += QString::number(decibels, 'f', precision);
        s += QStringLiteral(" dB");
        return s;
    }
    
}
