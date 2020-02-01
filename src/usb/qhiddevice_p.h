#ifndef QHIDDEVICE_P_H
#define QHIDDEVICE_P_H

#include "qhiddevice.h"
#include <private/qobject_p.h>
#include <hidapi.h>

QT_BEGIN_NAMESPACE

class QHidDevicePrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QHidDevice)

public:
    QHidDevicePrivate();
    ~QHidDevicePrivate();

    hid_device *m_devHandle;
};

QT_END_NAMESPACE

#endif // QHIDDEVICE_P_H
