// SPDX-FileCopyrightText: 2026 CrSjimo
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TALCSJACKGLOBAL_H
#define TALCSJACKGLOBAL_H

#include <QtGlobal>

#ifdef TALCSJACK_LIBRARY
#    define TALCSJACK_EXPORT Q_DECL_EXPORT
#else
#    define TALCSJACK_EXPORT Q_DECL_IMPORT
#endif

#endif
