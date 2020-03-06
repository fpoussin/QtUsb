#include "usbexample.h"
#include <QDebug>

UsbExample::UsbExample(QObject *parent)
    : QObject(parent)
{
    qDebug() << m_usb_info.getPresentDevices();

    exit(0);
}
