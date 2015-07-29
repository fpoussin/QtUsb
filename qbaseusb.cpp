#include "qbaseusb.h"


QBaseUsb::QBaseUsb(QObject *parent) :
    QThread(parent)
{
    this->setDefaults();
    mSpd = QUSB::unknownSpeed;
    mStop = false;

    connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
    this->start();
}

QBaseUsb::~QBaseUsb()
{
    mStop = true;
    this->wait();
}

quint16 QBaseUsb::getTimeout(void)
{
    return mTimeout;
}

void QBaseUsb::setDebug(bool enable)
{
    mDebug = enable;
}

bool QBaseUsb::addDevice(const QUSB::device& dev)
{
    if (this->findDevice(dev) == -1)
    {
        mDeviceList.append(dev);
        return true;
    }
    return false;
}

bool QBaseUsb::removeDevice(const QUSB::device &dev)
{
    const int pos = this->findDevice(dev);
    if (pos > 0)
    {
        mDeviceList.remove(pos);
        return true;
    }
    return true;
}

int QBaseUsb::findDevice(const QUSB::device &filter, QUSB::device* dev)
{
    for (int i = 0; i <= mDeviceList.length(); i++)
    {
       QUSB::device* d = &mDeviceList[i];

       if((d->guid == filter.guid) ||
               (d->pid == filter.pid && d->vid == filter.vid))
       {
           if (dev != NULL)
              *dev = *d;
           return i;
       }
    }

    return -1;
}

void QBaseUsb::setTimeout(quint16 timeout)
{
    mTimeout = timeout;
}

void QBaseUsb::setEndPoints(quint8 in, quint8 out)
{
    mDevice.readEp = in | 0x80;
    mDevice.writeEp = out;
}

void QBaseUsb::setDeviceIds(quint16 pid, quint16 vid)
{
    mDevice.pid = pid;
    mDevice.vid = vid;
}

bool QBaseUsb::setGuid(const QString &guid)
{
    (void)guid;
    return false;
}

void QBaseUsb::setConfiguration(quint8 config)
{
    mConfig = config;
}

void QBaseUsb::setInterface(quint8 interface)
{
    mDevice.interface = interface;
}

void QBaseUsb::showSettings()
{
    qWarning() << "\n"
               << "mDebug" << mDebug << "\n"
               << "mConfig" << mDevice.config << "\n"
               << "mTimeout" << mTimeout << "\n"
               << "mReadEp" << QString::number(mDevice.readEp, 16) << "\n"
               << "mWriteEp" << QString::number(mDevice.writeEp, 16) << "\n"
               << "mInterface" << mDevice.interface << "\n"
               << "mDevice.pid" << QString::number(mDevice.pid, 16) << "\n"
               << "mDevice.vid" << QString::number(mDevice.vid, 16) << "\n"
               << "mGuid" << mGuid;
}

void QBaseUsb::setDefaults()
{
    mConnected = false;
    mDebug = false;
    mTimeout = QUSB::DEFAULT_TIMEOUT_MSEC;
    mDevice.readEp = 0x81;
    mDevice.writeEp = 0x01;
    mDevice.config = 0x01;
    mDevice.interface = 0x00;
    mDevice.alternate = 0x00;
}

void QBaseUsb::monitorDevices()
{

}

void QBaseUsb::run()
{
    while (!mStop)
    {
        this->monitorDevices();
        this->msleep(100);
    }
}
