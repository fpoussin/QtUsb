#include "qbaseusb.h"


QBaseUsb::QBaseUsb(QObject *parent) :
    QObject(parent)
{

    this->setDefaults();
}

QBaseUsb::~QBaseUsb()
{

}

quint16 QBaseUsb::getTimeout(void)
{
    return this->mTimeout;
}

void QBaseUsb::setDebug(bool enable)
{
    this->mDebug = enable;
}

void QBaseUsb::setTimeout(quint16 timeout)
{
    this->mTimeout = timeout;
}

void QBaseUsb::setEndPoints(quint8 in, quint8 out)
{
    this->mReadEp = in | 0x80;
    this->mWriteEp = out;
}

void QBaseUsb::setDeviceIds(quint16 pid, quint16 vid)
{
    this->mPid = pid;
    this->mVid = vid;
}

bool QBaseUsb::setGuid(QString guid)
{
    (void)guid;
    return false;
}

void QBaseUsb::setConfiguration(quint8 config)
{
    this->mConfig = config;
}

void QBaseUsb::setInterface(quint8 interface)
{
    this->mInterface = interface;
}

void QBaseUsb::setDefaults()
{
    this->mDebug = false;
    this->mTimeout = DEFAULT_TIMEOUT_MSEC;
    this->mReadEp = 0x81;
    this->mWriteEp = 0x01;
    this->mConfig = 0;
    this->mInterface = 0;
}
