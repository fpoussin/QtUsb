#ifndef Q_USB_P_H
#define Q_USB_P_H

#include "qusb.h"
#include <private/qobject_p.h>
#include <QTimer>

#ifdef Q_OS_UNIX
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
