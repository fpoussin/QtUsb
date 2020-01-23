#ifndef QHIDDEVICE_H
#define QHIDDEVICE_H

#include <QObject>
#include "qusbdevice.h"

QT_BEGIN_NAMESPACE

class QHidDevicePrivate;

class QHidDevice : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QHidDevice)

public:
    explicit QHidDevice(QObject *parent = Q_NULLPTR);

private:
    QHidDevicePrivate *const d_dummy;
    Q_DISABLE_COPY(QHidDevice)
};

QT_END_NAMESPACE

#endif // QHIDDEVICE_H
