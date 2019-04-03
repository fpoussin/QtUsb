#ifndef Q_USB_INFO_P_H
#define Q_USB_INFO_P_H

#include "qusbinfo.h"
#include <private/qobject_p.h>
#include <QTimer>

#ifdef Q_OS_UNIX
#include <libusb-1.0/libusb.h>
#else
#include <libusb/libusb.h>
#endif

QT_BEGIN_NAMESPACE

class QUsbInfoPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QUsbInfo)

public:
    QUsbInfoPrivate();
    ~QUsbInfoPrivate();

    bool m_has_hotplug;
    libusb_context *m_ctx;
    QTimer *m_refresh_timer;
};

QT_END_NAMESPACE

#endif
