/******************************************************************************
* Copyright (c) 2024 CrSjimo                                                 *
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

#include "TakeOwnershipPointer.h"

namespace talcs {

    /**
     * @class TakeOwnershipPointer
     * @brief A helper smart pointer for taking ownership.
     *
     * It is suggested not to use this feature. Instead, you shall manage the objects on your own.
     * But for historical reasons, this feature is still kept.
     */

    /**
     * @fn TakeOwnershipPointer::TakeOwnershipPointer(T *ptr, bool takeOwnership)
     * Constructor.
     */

    /**
     * @fn TakeOwnershipPointer::~TakeOwnershipPointer()
     * Destructor. If takeOwnership is true, the pointer will be deleted.
     */

    /**
     * @fn void TakeOwnershipPointer::reset(T *ptr, bool takeOwnership)
     * Resets the pointer. Note that this does nothing on the old pointer no matter takeOwnership was
     * true or false.
     */

    /**
     * @fn T TakeOwnershipPointer::*get() const
     * Gets the pointer.
     */

    /**
     * @fn T *TakeOwnershipPointer::operator->() const
     * Smart pointer operator.
     */

    /**
     * @fn T &TakeOwnershipPointer::operator*() const
     * Smart pointer operator.
     */

    /**
     * @fn TakeOwnershipPointer::operator T*() const
     * Implicit conversion for compatibility.
     */

}