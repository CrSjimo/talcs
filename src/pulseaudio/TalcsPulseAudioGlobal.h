// SPDX-FileCopyrightText: 2026 CrSjimo
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TALCSPULSEAUDIOGLOBAL_H
#define TALCSPULSEAUDIOGLOBAL_H

#include <QtGlobal>

#ifdef TALCSPULSEAUDIO_LIBRARY
#    define TALCSPULSEAUDIO_EXPORT Q_DECL_EXPORT
#else
#    define TALCSPULSEAUDIO_EXPORT Q_DECL_IMPORT
#endif

#endif
