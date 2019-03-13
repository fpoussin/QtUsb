#ifndef QUSBDEVICE_P_H
#define QUSBDEVICE_P_H

#include "qusbdevice.h"
#include <private/qobject_p.h>
#include <libusb-1.0/libusb.h>

QT_BEGIN_NAMESPACE

class QUsbDevicePrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QUsbDevice)
public:
    QUsbDevicePrivate();

private:


};

QT_END_NAMESPACE

#endif
