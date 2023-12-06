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

#include "ErrorStringProvider.h"

namespace talcs {

    /**
     * @class ErrorStringProvider
     * @brief Base class that supports setting and getting the error string.
     */

    /**
     * Gets the error string.
     */
    QString ErrorStringProvider::errorString() const {
        return m_error;
    }

    /**
     * Sets the error string.
     */
    void ErrorStringProvider::setErrorString(const QString &error) {
        this->m_error = error;
    }

    /**
     * Clears the error string.
     */
    void ErrorStringProvider::clearErrorString() {
        this->m_error.clear();
    }
    
}
