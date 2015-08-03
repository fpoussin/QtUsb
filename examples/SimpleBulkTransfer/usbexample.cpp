#include "usbexample.h"

UsbExample::UsbExample(QObject *parent) :
    QObject(parent)
{
    this->setupDevice();

    QByteArray send, recv;

    send.append(0xAB);

    #ifdef interface
    #undef interface
    #endif

    if (this->openDevice())
    {
        qDebug("Device open!");
        this->write(&send);
        this->read(&recv);
    }
}

UsbExample::~UsbExample()
{
    delete mUsbDev;
}

void UsbExample::setupDevice()
{
    /* There are 2 ways of identifying devices depending on the platform.
     * You can use both methods, only one will be taken into account.
     */

    mUsbDev = new QUsbDevice();
    mUsbDev->setDebug(true);

    // Linux
    mFilter.pid = 0x3748;
    mFilter.vid = 0x0483;

    // Win (Get this id from the driver's .inf)
    mFilter.guid = "DBCE1CD9-A320-4b51-A365-A0C3F3C5FB29";

    //
    mConfig.alternate = 0;
    mConfig.config = 1;
    mConfig.interface = 1;
    mConfig.readEp = 0x81;
    mConfig.writeEp = 0x02;
}

bool UsbExample::openDevice()
{
    qDebug("Opening");

    QtUsb::DeviceStatus ds;
    ds = mUsbManager.openDevice(mUsbDev, mFilter, mConfig);

    if (ds == QtUsb::deviceOK) {
        // Device is open
        return true;
    }
    return false;
}

bool UsbExample::closeDevice()
{
    qDebug("Closing");
    mUsbManager.closeDevice(mUsbDev);
    return false;
}

void UsbExample::read(QByteArray *buf)
{
    *buf = mUsbDev->read(1);
}

void UsbExample::write(QByteArray *buf)
{
    mUsbDev->write(*buf);
}
