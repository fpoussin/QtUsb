#include "usbexample.h"

#ifdef interface
#undef interface
#endif

UsbExample::UsbExample(QObject *parent) :
    QObject(parent)
{
    this->setupDevice();

    QByteArray send, recv;

    send.append((char)0xAB);

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

    //
    mFilter.pid = 0x3748;
    mFilter.vid = 0x0483;

    //
    mConfig.alternate = 0;
    mConfig.config = 0;
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
    mUsbDev->read(buf, 1);
}

void UsbExample::write(QByteArray *buf)
{
    mUsbDev->write(buf, buf->size());
}
