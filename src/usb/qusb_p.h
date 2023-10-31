#ifndef Q_USB_P_H
#define Q_USB_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qusb.h"
#include <private/qobject_p.h>
#include <QTimer>

#if defined(Q_OS_MACOS)
  #include <libusb.h>
  #include <hidapi.h>
#elif defined(Q_OS_UNIX)
  #include <libusb-1.0/libusb.h>
  #include <hidapi.h>
#else
  #include <libusb/libusb.h>
  #include <hidapi/hidapi.h>
#endif

QT_BEGIN_NAMESPACE

class QUsbPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QUsb)

public:
    QUsbPrivate();
    ~QUsbPrivate();

    bool m_has_hotplug;
    libusb_context *m_ctx;
    QTimer *m_refresh_timer;
};

QT_END_NAMESPACE

#endif
