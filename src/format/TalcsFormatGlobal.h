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

#ifndef TALCSFORMATGLOBAL_H
#define TALCSFORMATGLOBAL_H

#include <QtGlobal>

#ifndef TALCSFORMAT_EXPORT
#  ifdef TALCSFORMAT_STATIC
#    define TALCSFORMAT_EXPORT
#  else
#    ifdef TALCSFORMAT_LIBRARY
#      define TALCSFORMAT_EXPORT Q_DECL_EXPORT
#    else
#      define TALCSFORMAT_EXPORT Q_DECL_IMPORT
#    endif
#  endif
#endif

#endif // TALCSFORMATGLOBAL_H
