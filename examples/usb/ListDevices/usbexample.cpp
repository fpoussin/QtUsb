#include "usbexample.h"
#include <QDebug>

UsbExample::UsbExample(QObject *parent)
    : QObject(parent)
{
    auto list = m_usb_info.getPresentDevices();

    qDebug() << list.size() << "devices present";
    qDebug() << list;

    exit(0);
}
