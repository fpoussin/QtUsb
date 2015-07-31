#include "usbexample.h"

UsbExample::UsbExample(QObject *parent) :
    QObject(parent)
{
    QObject::connect(&mUsbManager, SIGNAL(deviceInserted(QtUsb::FilterList)), this, SLOT(onDevInserted(QtUsb::FilterList)));
    QObject::connect(&mUsbManager, SIGNAL(deviceRemoved(QtUsb::FilterList)), this, SLOT(onDevRemoved(QtUsb::FilterList)));

    qDebug("Starting");
}

UsbExample::~UsbExample()
{

}

void UsbExample::onDevInserted(QtUsb::FilterList list)
{
    qDebug("devices inserted");
    for (int i = 0; i < list.length(); i++)
    {
        QtUsb::DeviceFilter f = list.at(i);
        qDebug("%04x:%04x", f.vid, f.pid);
    }
}

void UsbExample::onDevRemoved(QtUsb::FilterList list)
{
    qDebug("devices removed");
    for (int i = 0; i < list.length(); i++)
    {
        QtUsb::DeviceFilter f = list.at(i);
        qDebug("%04x:%04x", f.vid, f.pid);
    }
}

