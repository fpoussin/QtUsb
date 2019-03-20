#ifndef QUSB_GLOBAL_H
#define QUSB_GLOBAL_H

#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

#ifndef QT_STATIC
#if defined(QT_BUILD_USB_LIB)
#define Q_USB_EXPORT Q_DECL_EXPORT
#else
#define Q_USB_EXPORT Q_DECL_IMPORT
#endif
#else
#define Q_USB_EXPORT
#endif

QT_END_NAMESPACE

#endif // QUSB_GLOBAL_H
