#include "usbexample.h"

UsbExample::UsbExample(QObject *parent) :
    QObject(parent)
{
    mUsb.setDebug(true);
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

    // Linux
    mUsb.setDeviceIds(0x1234, 0xABCD);

    // Win (Get this id from the driver's .inf)
    mUsb.setGuid("");


    mUsb.setEndPoints(USB_PIPE_IN, USB_PIPE_OUT);
}

bool UsbExample::openDevice()
{
    qint32 open = mUsb.open();
    if ((open >= 0)) {
        // Device is open

        return true;
    }
    return false;
}

bool UsbExample::closeDevice()
{
    mUsb.close();
    qDebug("Closing");
    return false;
}

void UsbExample::read(QByteArray *buf)
{
    mUsb.read(buf, 1);
}

void UsbExample::write(QByteArray *buf)
{
    mUsb.write(buf, buf->length());
}
