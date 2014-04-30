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
    mReadEp = in | 0x80;
    mWriteEp = out;
}

void QBaseUsb::setDeviceIds(quint16 pid, quint16 vid)
{
    mPid = pid;
    mVid = vid;
}

bool QBaseUsb::setGuid(const QString &guid)
{
    (void)guid;
    qWarning() << "setGuid Dummy";
    return false;
}

void QBaseUsb::setConfiguration(quint8 config)
{
    mConfig = config;
}

void QBaseUsb::setInterface(quint8 interface)
{
    mInterface = interface;
}

void QBaseUsb::showSettings()
{
    qWarning() << "\n"
               << "mDebug" << mDebug << "\n"
               << "mConfig" << mConfig << "\n"
               << "mTimeout" << mTimeout << "\n"
               << "mReadEp" << QString::number(mReadEp, 16) << "\n"
               << "mWriteEp" << QString::number(mWriteEp, 16) << "\n"
               << "mInterface" << mInterface << "\n"
               << "mPid" << QString::number(mPid, 16) << "\n"
               << "mVid" << QString::number(mVid, 16) << "\n"
               << "mGuid" << mGuid;
}

void QBaseUsb::setDefaults()
{
    mDebug = false;
    mTimeout = DEFAULT_TIMEOUT_MSEC;
    mReadEp = 0x81;
    mWriteEp = 0x01;
    mConfig = 0;
    mInterface = 0;
    mAlternate = 0;
}
