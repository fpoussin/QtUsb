#include "qbaseusb.h"


QBaseUsb::QBaseUsb(QObject *parent) :
    QThread(parent)
{
    this->setDefaults();
    this->mSpd = QUSB::unknownSpeed;
    this->mStop = false;

    connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
    this->start();
}

QBaseUsb::~QBaseUsb()
{

}

quint16 QBaseUsb::getTimeout(void)
{
    return mTimeout;
}

void QBaseUsb::setDebug(bool enable)
{
    mDebug = enable;
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
