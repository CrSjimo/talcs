// SPDX-FileCopyrightText: 2026 CrSjimo
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TALCSPIPEWIREGLOBAL_H
#define TALCSPIPEWIREGLOBAL_H

#include <QtGlobal>

#ifdef TALCSPIPEWIRE_LIBRARY
#    define TALCSPIPEWIRE_EXPORT Q_DECL_EXPORT
#else
#    define TALCSPIPEWIRE_EXPORT Q_DECL_IMPORT
#endif

#endif
