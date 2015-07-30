#ifndef QUSB_GLOBAL_H
#define QUSB_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(QUSB_LIBRARY)
#  define QUSBSHARED_EXPORT Q_DECL_EXPORT
#else
#  define QUSBSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // QUSB_GLOBAL_H
