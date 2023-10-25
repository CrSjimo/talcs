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
