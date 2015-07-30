#include "usbexample.h"

UsbExample::UsbExample(QObject *parent) :
    QObject(parent)
{
    mUsbDev.setDebug(true);
    this->setupDevice();

    QByteArray send, recv;

    send.append(0xAB);

    if (this->openDevice())
    {
        this->write(&send);
        this->read(&recv);
    }
}

UsbExample::~UsbExample()
{
    this->closeDevice();
}

void UsbExample::setupDevice()
{
    /* There are 2 ways of identifying devices depending on the platform.
     * You can use both methods, only one will be taken into account.
     */

    QtUsb::UsbDeviceDescription desc;

    // Linux
    desc.pid = 0x1234;
    desc.vid = 0x1234;

    // Win (Get this id from the driver's .inf)
    desc.guid = "";


    mUsbManager.addDevice(desc);

    mUsbDev.setEndPoints(USB_PIPE_IN, USB_PIPE_OUT);
}

bool UsbExample::openDevice()
{
    qint32 open = mUsbDev.open();
    if ((open >= 0)) {
        // Device is open

        return true;
    }
    return false;
}

bool UsbExample::closeDevice()
{
    mUsbDev.close();
    qDebug("Closing");
    return false;
}

void UsbExample::read(QByteArray *buf)
{
    mUsbDev.read(buf, 1);
}

void UsbExample::write(QByteArray *buf)
{
    mUsbDev.write(buf, buf->length());
}
