#ifndef QTSCHEMEGLOBAL_H
#define QTSCHEMEGLOBAL_H

#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

#ifndef Q_SCHEME_EXPORT
#  ifndef QT_STATIC
#    if defined(QT_BUILD_SCHEME_LIB)
#      define Q_SCHEME_EXPORT Q_DECL_EXPORT
#    else
#      define Q_SCHEME_EXPORT Q_DECL_IMPORT
#    endif
#  else
#    define Q_SCHEME_EXPORT
#  endif
#endif


#endif // QTSCHEMEGLOBAL_H
