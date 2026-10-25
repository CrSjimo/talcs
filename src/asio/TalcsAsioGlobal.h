/*******************************************************************************
 * Copyright (c) 2023 CrSjimo                                                  *
 * SPDX-License-Identifier: GPL-3.0-only                                       *
 *                                                                             *
 * This file is part of TALCS.                                                 *
 *                                                                             *
 * TALCS is free software: you can redistribute it and/or modify it under the  *
 * terms of the GNU General Public License as published by the Free Software   *
 * Foundation, version 3 of the License.                                       *
 *                                                                             *
 * TALCS is distributed in the hope that it will be useful, but WITHOUT ANY    *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS   *
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more       *
 * details.                                                                    *
 *                                                                             *
 * You should have received a copy of the GNU General Public License along     *
 * with TALCS. If not, see <https://www.gnu.org/licenses/>.                    *
 ******************************************************************************/

#ifndef TALCSASIOGLOBAL_H
#define TALCSASIOGLOBAL_H

#include <QtGlobal>

#ifndef TALCSASIO_EXPORT
#    ifdef TALCSASIO_STATIC
#        define TALCSASIO_EXPORT
#    else
#        ifdef TALCSASIO_LIBRARY
#            define TALCSASIO_EXPORT Q_DECL_EXPORT
#        else
#            define TALCSASIO_EXPORT Q_DECL_IMPORT
#        endif
#    endif
#endif

#endif // TALCSASIOGLOBAL_H
