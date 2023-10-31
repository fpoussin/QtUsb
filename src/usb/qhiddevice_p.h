#ifndef QHIDDEVICE_P_H
#define QHIDDEVICE_P_H

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
