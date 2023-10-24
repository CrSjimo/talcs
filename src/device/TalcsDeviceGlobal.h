#ifndef TALCSDEVICEGLOBAL_H
#define TALCSDEVICEGLOBAL_H

#include <QtGlobal>

#ifndef TALCSDEVICE_EXPORT
#  ifdef TALCSDEVICE_STATIC
#    define TALCSDEVICE_EXPORT
#  else
#    ifdef TALCSDEVICE_LIBRARY
#      define TALCSDEVICE_EXPORT Q_DECL_EXPORT
#    else
#      define TALCSDEVICE_EXPORT Q_DECL_IMPORT
#    endif
#  endif
#endif

#endif // TALCSDEVICEGLOBAL_H
