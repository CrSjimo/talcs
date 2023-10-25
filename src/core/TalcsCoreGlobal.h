#ifndef TALCSCOREGLOBAL_H
#define TALCSCOREGLOBAL_H

#include <QtGlobal>

#ifndef TALCSCORE_EXPORT
#  ifdef TALCSCORE_STATIC
#    define TALCSCORE_EXPORT
#  else
#    ifdef TALCSCORE_LIBRARY
#      define TALCSCORE_EXPORT Q_DECL_EXPORT
#    else
#      define TALCSCORE_EXPORT Q_DECL_IMPORT
#    endif
#  endif
#endif

#endif // TALCSCOREGLOBAL_H
