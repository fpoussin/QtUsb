#include "usbexample.h"

UsbExample::UsbExample(QObject *parent)
    : QObject(parent)
{
    QObject::connect(&m_usb_info, &QUsbInfo::deviceInserted,
                     this, &UsbExample::onDevInserted);
    QObject::connect(&m_usb_info, &QUsbInfo::deviceRemoved,
                     this, &UsbExample::onDevRemoved);

    qInfo("Starting...");
    qInfo("Press CTRL+C to close.");
}

UsbExample::~UsbExample()
{
    qInfo("Closing...");
}

void UsbExample::onDevInserted(QUsbDevice::Id id)
{
    qInfo("Device inserted: %04x:%04x", id.vid, id.pid);
}

void UsbExample::onDevRemoved(QUsbDevice::Id id)
{
    qInfo("Device removed: %04x:%04x", id.vid, id.pid);
}
