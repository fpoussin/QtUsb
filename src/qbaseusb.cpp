#include "qbaseusb.h"


QBaseUsbDevice::QBaseUsbDevice(QObject *parent) :
    QIODevice(parent)
{
    this->setDefaults();
    mSpd = QtUsb::unknownSpeed;
}

QBaseUsbDevice::~QBaseUsbDevice()
{

}

quint16 QBaseUsbDevice::getTimeout(void)
{
    return mTimeout;
}

void QBaseUsbDevice::setDebug(bool enable)
{
    mDebug = enable;
}

void QBaseUsbDevice::setTimeout(quint16 timeout)
{
    mTimeout = timeout;
}


void QBaseUsbDevice::showSettings()
{
    qWarning() << "\n"
               << "mDebug" << mDebug << "\n"
               << "mConfig" << mConfig.config << "\n"
               << "mTimeout" << mTimeout << "\n"
               << "mReadEp" << QString::number(mConfig.readEp, 16) << "\n"
               << "mWriteEp" << QString::number(mConfig.writeEp, 16) << "\n"
               << "mInterface" << mConfig.interface << "\n"
               << "mDevice.pid" << QString::number(mFilter.pid, 16) << "\n"
               << "mDevice.vid" << QString::number(mFilter.vid, 16) << "\n"
               << "mGuid" << mFilter.guid;
}

void QBaseUsbDevice::setDefaults()
{
    mConnected = false;
    mDebug = false;
    mTimeout = QtUsb::DefaultTimeout;
    mConfig.readEp = 0x81;
    mConfig.writeEp = 0x01;
    mConfig.config = 0x01;
    mConfig.interface = 0x00;
    mConfig.alternate = 0x00;
}

