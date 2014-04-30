#include "qlibusb.h"

QUsb::QUsb(QBaseUsb *parent) :
    QBaseUsb(parent)
{
    mDevHandle = NULL;
    int r = libusb_init(&mCtx); //initialize the library for the session we just declared
    if(r < 0) {
        qWarning() << "Init Error " << r; //there was an error
    }
}

QUsb::~QUsb()
{
    this->close();
}

qint32 QUsb::open()
{
    int r; // for return values
    ssize_t cnt; // holding number of devices in list

    cnt = libusb_get_device_list(mCtx, &mDevs); // get the list of devices
    if(cnt < 0) {
        qWarning() << "Get Device Error";
        return -1;
    }

    mDevHandle = libusb_open_device_with_vid_pid(mCtx, mVid, mPid); // Open device
    if(mDevHandle == NULL) {
        qWarning() << "Cannot open device";
        return -2;
    }
    if (mDebug) qDebug() << "Device Opened";
    libusb_free_device_list(mDevs, 1); // free the list, unref the devices in it

    if(libusb_kernel_driver_active(mDevHandle, mInterface) == 1) { // find out if kernel driver is attached
        if (mDebug) qDebug() << "Kernel Driver Active";
        if(libusb_detach_kernel_driver(mDevHandle, mInterface) == 0) // detach it
            if (mDebug) qDebug() << "Kernel Driver Detached!";
    }

    int conf;
    libusb_get_configuration(mDevHandle, &conf);

    if (conf != mConfig) {
        r = libusb_set_configuration(mDevHandle, mConfig);
        if(r != 0) {
            qWarning() << "Cannot Set Configuration";
            return -3;
        }
    }
    r = libusb_claim_interface(mDevHandle, mInterface);
    if(r != 0) {
        qWarning() << "Cannot Claim Interface";
        return -4;
    }

    return 0;
}

void QUsb::close()
{
    if (mDevHandle) {
        // stop any further write attempts whilst we close down
        qDebug() << "Closing USB connection...";

        libusb_release_interface(mDevHandle, 0); //release the claimed interface
        libusb_close(mDevHandle); //close the device we opened
        libusb_exit(mCtx); //needs to be called to end the
    }
}

qint32 QUsb::read(QByteArray *buf, quint32 bytes)
{
    qint32 rc, actual, actual_tmp;
    QElapsedTimer timer;

    // check it isn't closed already
    if (!mDevHandle) return -1;

    if (bytes == 0)
        return 0;

    actual = 0;
    actual_tmp = 0;
    uchar *buffer = new uchar[bytes];

    timer.start();
    while (timer.elapsed() < mTimeout && bytes-actual > 0) {
        rc = libusb_bulk_transfer(mDevHandle, (mReadEp), buffer+actual, bytes-actual, &actual_tmp, mTimeout);
        actual += actual_tmp;
        if (rc != 0) break;
    }
    // we clear the buffer.
    buf->clear();
    QString data, s;

    for (qint32 i = 0; i < actual; i++) {
        buf->append(buffer[i]);
        if (mDebug) data.append(s.sprintf("%02X",(uchar)buf->at(i))+":");
    }
    if (mDebug) {
        data.remove(data.size()-1, 1); //remove last colon
        qDebug() << "Received: " << data;
    }

    delete buffer;
    if (rc != 0)
    {
        if (rc == -110)
            qWarning() << "libusb_bulk_transfer Timeout";
        else
            qWarning() << "libusb_bulk_transfer Error reading: " << rc;
        return rc;
    }

    return actual;
}

qint32 QUsb::write(QByteArray *buf, quint32 bytes)
{
    qint32 rc, actual, actual_tmp;
    QElapsedTimer timer;

    // check it isn't closed
    if (!mDevHandle) return -1;

    if (mDebug) {
        QString cmd, s;
        for (int i=0; i<buf->size(); i++) {
            cmd.append(s.sprintf("%02X",(uchar)buf->at(i))+":");
        }
        cmd.remove(cmd.size()-1, 1); //remove last colon
        qDebug() << "Sending" << buf->size() << "bytes:" << cmd;
    }

    actual = 0;
    actual_tmp = 0;

    timer.start();
    while (timer.elapsed() < mTimeout && bytes-actual > 0) {
        rc = libusb_bulk_transfer(mDevHandle, (mWriteEp), (uchar*)buf->constData(), bytes, &actual, mTimeout);
        actual += actual_tmp;
        if (rc != 0) break;
    }

    if (rc != 0)
    {
        if (rc == -110)
            qWarning() << "libusb_bulk_transfer Timeout";
        else if (rc == -2)
            qWarning() << "EndPoint not found";
        else
            qWarning() << "libusb_bulk_transfer Error Writing: "<< rc;
    }

    return actual;
}

void QUsb::setDebug(bool enable)
{
    QBaseUsb::setDebug(enable);
    if (enable)
        libusb_set_debug(mCtx, 3);
    else
        libusb_set_debug(mCtx, 0);
}
