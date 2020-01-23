#include "qhiddevice.h"
#include "qhiddevice_p.h"

QHidDevice::QHidDevice(QObject *parent)
    : QObject(*(new QHidDevicePrivate), parent), d_dummy(Q_NULLPTR)
{

}

QHidDevicePrivate::QHidDevicePrivate()
{
    int res;

    res = hid_init();
}
