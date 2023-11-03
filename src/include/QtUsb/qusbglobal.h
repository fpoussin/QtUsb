#ifndef Q_USB_GLOBAL_H
#define Q_USB_GLOBAL_H

#include <QtCore>

#if !defined(QT_STATIC)
#	if defined(QT_BUILD_USB_LIB)
#		define Q_USB_EXPORT Q_DECL_EXPORT
#	else
#		define Q_USB_EXPORT Q_DECL_IMPORT
#	endif
#else
#	define Q_USB_EXPORT
#endif

#endif
