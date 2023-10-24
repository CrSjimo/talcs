#ifndef TALCSREMOTEGLOBAL_H
#define TALCSREMOTEGLOBAL_H

#include <QtGlobal>

#ifndef TALCSREMOTE_EXPORT
#  ifdef TALCSREMOTE_STATIC
#    define TALCSREMOTE_EXPORT
#  else
#    ifdef TALCSREMOTE_LIBRARY
#      define TALCSREMOTE_EXPORT Q_DECL_EXPORT
#    else
#      define TALCSREMOTE_EXPORT Q_DECL_IMPORT
#    endif
#  endif
#endif

#endif // TALCSREMOTEGLOBAL_H
