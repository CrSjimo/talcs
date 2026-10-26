// SPDX-FileCopyrightText: 2026 CrSjimo
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TALCSALSAGLOBAL_H
#define TALCSALSAGLOBAL_H

#include <QtGlobal>

#ifdef TALCSALSA_LIBRARY
#    define TALCSALSA_EXPORT Q_DECL_EXPORT
#else
#    define TALCSALSA_EXPORT Q_DECL_IMPORT
#endif

#endif
