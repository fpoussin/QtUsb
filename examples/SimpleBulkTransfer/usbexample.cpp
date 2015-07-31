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

    // Linux
    mFilter.pid = 0x1234;
    mFilter.vid = 0xABCD;

    // Win (Get this id from the driver's .inf)
    mFilter.guid = "";
}

bool UsbExample::openDevice()
{
    qDebug("Opening");

    QtUsb::DeviceStatus ds;
    ds = mUsbManager.openDevice(&mUsbDev, mFilter, mConfig);

    if (ds == QtUsb::deviceOK) {
        // Device is open
        mUsbDev.setConfig(mConfig);
        return true;
    }
    return false;
}

bool UsbExample::closeDevice()
{
    qDebug("Closing");
    mUsbManager.closeDevice(&mUsbDev);
    return false;
}

void UsbExample::read(QByteArray *buf)
{
    *buf = mUsbDev.read(1);
}

void UsbExample::write(QByteArray *buf)
{
    mUsbDev.write(*buf);
}
