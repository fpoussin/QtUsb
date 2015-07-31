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

void UsbExample::addDevice()
{
    QtUsb::DeviceFilter filter;

    filter.pid = 0x1234;
    filter.vid = 0xABCD;

    mUsbManager.addDevice(filter);
}

void UsbExample::onDevInserted(QtUsb::FilterList list)
{
    qDebug() << "devices inserted";
    QString str;
    for (int i = 0; i < list.length(); i++)
    {
        QtUsb::DeviceFilter f = list.at(i);
        qDebug(str.sprintf("%04x:%04x", f.vid, f.pid).toStdString().c_str());
    }
}

void UsbExample::onDevRemoved(QtUsb::FilterList list)
{
    qDebug() << "devices removed";
    QString str;
    for (int i = 0; i < list.length(); i++)
    {
        QtUsb::DeviceFilter f = list.at(i);
        qDebug(str.sprintf("%04x:%04x", f.vid, f.pid).toStdString().c_str());
    }
}

