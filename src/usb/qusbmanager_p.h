#ifndef Q_USB_MANAGER_P_H
#define Q_USB_MANAGER_P_H

#include "qusbmanager.h"
#include <libusb-1.0/libusb.h>
#include <private/qobject_p.h>

QT_BEGIN_NAMESPACE

class QUsbManagerPrivate : public QObjectPrivate {
  Q_DECLARE_PUBLIC(QUsbManager)
public:
  QUsbManagerPrivate();

private:
  libusb_context *m_ctx;
};

QT_END_NAMESPACE

#endif
