#include "usbexample.h"

UsbExample::UsbExample(QObject *parent)
    : QObject(parent)
{
    QObject::connect(&m_usb, &QUsb::deviceInserted,
                     this, &UsbExample::onDevInserted);
    QObject::connect(&m_usb, &QUsb::deviceRemoved,
                     this, &UsbExample::onDevRemoved);

    qInfo("Starting...");
    qInfo("Press CTRL+C to close.");
}

UsbExample::~UsbExample()
{
    qInfo("Closing...");
}

void UsbExample::onDevInserted(QUsb::Id id)
{
    qInfo("Device inserted: %04x:%04x", id.vid, id.pid);
}

void UsbExample::onDevRemoved(QUsb::Id id)
{
    qInfo("Device removed: %04x:%04x", id.vid, id.pid);
}
