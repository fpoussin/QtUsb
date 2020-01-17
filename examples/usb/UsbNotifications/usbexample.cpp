#include "usbexample.h"

UsbExample::UsbExample(QObject *parent)
    : QObject(parent)
{
    QObject::connect(&m_usb_info, SIGNAL(deviceInserted(QUsbDevice::IdList)),
                     this, SLOT(onDevInserted(QUsbDevice::IdList)));
    QObject::connect(&m_usb_info, SIGNAL(deviceRemoved(QUsbDevice::IdList)), this,
                     SLOT(onDevRemoved(QUsbDevice::IdList)));

    m_usb_info.setLogLevel(QUsbDevice::logDebug);
    qInfo("Starting...");
    qInfo("Press CTRL+C to close.");
}

UsbExample::~UsbExample()
{
    qInfo("Closing...");
}

void UsbExample::onDevInserted(QUsbDevice::IdList list)
{
    qInfo("devices inserted");
    for (int i = 0; i < list.length(); i++) {
        QUsbDevice::Id f = list.at(i);
        qInfo("V%04x:P%04x", f.vid, f.pid);
    }
}

void UsbExample::onDevRemoved(QUsbDevice::IdList list)
{
    qInfo("devices removed");
    for (int i = 0; i < list.length(); i++) {
        QUsbDevice::Id f = list.at(i);
        qInfo("V%04x:P%04x", f.vid, f.pid);
    }
}
