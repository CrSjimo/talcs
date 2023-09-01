#ifndef TALCSGLOBAL_H
#define TALCSGLOBAL_H

#include <QtGlobal>

#ifndef TALCS_EXPORT
#  ifdef TALCS_STATIC
#    define TALCS_EXPORT
#  else
#    ifdef TALCS_LIBRARY
#      define TALCS_EXPORT Q_DECL_EXPORT
#    else
#      define TALCS_EXPORT Q_DECL_IMPORT
#    endif
#  endif
#endif

#endif // TALCSGLOBAL_H
