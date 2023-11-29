#ifndef TALCSMIDIGLOBAL_H
#define TALCSMIDIGLOBAL_H

#include <QtGlobal>

#ifndef TALCSMIDI_EXPORT
#  ifdef TALCSMIDI_STATIC
#    define TALCSMIDI_EXPORT
#  else
#    ifdef TALCSMIDI_LIBRARY
#      define TALCSMIDI_EXPORT Q_DECL_EXPORT
#    else
#      define TALCSMIDI_EXPORT Q_DECL_IMPORT
#    endif
#  endif
#endif

#endif // TALCSMIDIGLOBAL_H
