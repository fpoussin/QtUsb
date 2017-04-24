#include "qlibusb.h"

QUsbDevice::QUsbDevice(QBaseUsbDevice* parent) : QBaseUsbDevice(parent) {
  mDevHandle = NULL;
  int r = libusb_init(
      &mCtx);  // initialize the library for the session we just declared
  if (r < 0) {
    qCritical("LibUsb Init Error %d", r);  // there was an error
  }
  mReadBufferSize = 1024 * 64;
}

QUsbDevice::~QUsbDevice() {
  this->close();
  libusb_exit(mCtx);
}

QtUsb::FilterList QUsbDevice::getAvailableDevices() {
  QtUsb::FilterList list;
  ssize_t cnt;  // holding number of devices in list
  libusb_device** devs;
  libusb_context* ctx;

  libusb_init(&ctx);
  cnt = libusb_get_device_list(ctx, &devs);  // get the list of devices
  if (cnt < 0) {
    qCritical("libusb_get_device_list Error");
    libusb_free_device_list(devs, 1);
    return list;
  }

  for (int i = 0; i < cnt; i++) {
    libusb_device* dev = devs[i];
    libusb_device_descriptor desc;

    if (libusb_get_device_descriptor(dev, &desc) == 0) {
      QtUsb::DeviceFilter filter;
      filter.pid = desc.idProduct;
      filter.vid = desc.idVendor;

      list.append(filter);
    }
  }

  libusb_free_device_list(devs, 1);
  libusb_exit(ctx);
  return list;
}

qint32 QUsbDevice::open() {
  UsbPrintFuncName();

  int rc = -5;   // Not found by default
  ssize_t cnt;  // holding number of devices in list
  libusb_device* dev = NULL;

  if (mConnected) return -1;

  cnt = libusb_get_device_list(mCtx, &mDevs);  // get the list of devices
  if (cnt < 0) {
    qCritical("libusb_get_device_list error");
    libusb_free_device_list(mDevs, 1);
    return -1;
  }

  for (int i = 0; i < cnt; i++) {
    dev = mDevs[i];
    libusb_device_descriptor desc;

    if (libusb_get_device_descriptor(dev, &desc) == 0) {
      if (desc.idProduct == mFilter.pid && desc.idVendor == mFilter.vid) {
        if (mDebug) {
          qDebug("Found device.");
        }
        rc = libusb_open(dev, &mDevHandle);
        if (rc == 0) break;
        else {
          qWarning("Failed to open device: %s", libusb_strerror((enum libusb_error)rc));
        }
      }
    }
  }
  libusb_free_device_list(mDevs, 1);  // free the list, unref the devices in it

  if (rc != 0 || mDevHandle == NULL) {
    return rc;
  }

  if (mDebug) qDebug("Device Opened");

  if (libusb_kernel_driver_active(mDevHandle, mConfig.interface) ==
      1) {  // find out if kernel driver is attached
    if (mDebug) qDebug("Kernel Driver Active");
    if (libusb_detach_kernel_driver(mDevHandle, mConfig.interface) ==
        0)  // detach it
      if (mDebug) qDebug("Kernel Driver Detached!");
  }

  int conf;
  libusb_get_configuration(mDevHandle, &conf);

  if (conf != mConfig.config) {
    if (mDebug) qDebug("Configuration needs to be changed");
    rc = libusb_set_configuration(mDevHandle, mConfig.config);
    if (rc != 0) {
      qWarning("Cannot Set Configuration");
      this->printUsbError(rc);
      return -3;
    }
  }
  rc = libusb_claim_interface(mDevHandle, mConfig.interface);
  if (rc != 0) {
    qWarning("Cannot Claim Interface");
    this->printUsbError(rc);
    return -4;
  }

  switch (libusb_get_device_speed(dev)) {
    case LIBUSB_SPEED_LOW:
      this->mSpd = QtUsb::lowSpeed;
      break;
    case LIBUSB_SPEED_FULL:
      this->mSpd = QtUsb::fullSpeed;
      break;
    case LIBUSB_SPEED_HIGH:
      this->mSpd = QtUsb::highSpeed;
      break;
    case LIBUSB_SPEED_SUPER:
      this->mSpd = QtUsb::superSpeed;
      break;
    default:
      this->mSpd = QtUsb::unknownSpeed;
      break;
  }

  mConnected = true;

  return 0;
}

void QUsbDevice::close() {
  UsbPrintFuncName();

  if (mDevHandle && mConnected) {
    // stop any further write attempts whilst we close down
    qDebug("Closing USB connection...");

    QBaseUsbDevice::close();

    libusb_release_interface(mDevHandle, 0);  // release the claimed interface
    libusb_close(mDevHandle);                 // close the device we opened
  }

  mConnected = false;
}

void QUsbDevice::setDebug(bool enable) {
  QBaseUsbDevice::setDebug(enable);
  if (enable)
    libusb_set_debug(mCtx, LIBUSB_LOG_LEVEL_INFO);
  else
    libusb_set_debug(mCtx, LIBUSB_LOG_LEVEL_ERROR);
}

void QUsbDevice::printUsbError(int error_code)
{
    qWarning("libusb Error: %s", libusb_strerror((enum libusb_error)error_code));
}

void QUsbDevice::flush() {
  QByteArray buf;
  qint32 read_bytes;

  buf.resize(4096);
  libusb_bulk_transfer(mDevHandle, mConfig.readEp, (uchar*)(buf.data()), 4096,
                       &read_bytes, 25);
}

qint32 QUsbDevice::read(QByteArray* buf, quint32 len) {
  UsbPrintFuncName();
  Q_CHECK_PTR(buf);
  qint32 rc, read_bytes;
  qint32 read_total;
  QElapsedTimer timer;

  // check it isn't closed already
  if (!mDevHandle || !mConnected) return -1;

  if (len == 0) return 0;

  read_total = 0;
  read_bytes = 0;

  if (mReadBuffer.isEmpty())
    if (mDebug) qDebug("Read cache empty");

  /* Fetch from buffer first */
  if (len <= (quint32)mReadBuffer.size() && !mReadBuffer.isEmpty()) {
    if (mDebug) qDebug("Reading %d bytes from cache", mReadBuffer.size());
    *buf = mReadBuffer.mid(0, len);
    mReadBuffer.remove(0, len);
    return len;
  }

  /* Copy what's in the read buffer */
  len -= mReadBuffer.size();
  *buf = mReadBuffer;
  mReadBuffer.clear();
  mReadBuffer.resize(mReadBufferSize);

  /* Wait till we have at least the required data */
  timer.start();
  rc = LIBUSB_SUCCESS;
  while (timer.elapsed() < mTimeout && (qint32)len - read_total > 0) {
    rc = libusb_bulk_transfer(mDevHandle, mConfig.readEp,
                              (uchar*)(mReadBuffer.data() + read_total),
                              mReadBufferSize, &read_bytes, 10);
    read_total += read_bytes;
    if (rc == LIBUSB_ERROR_PIPE) {
      libusb_clear_halt(mDevHandle, mConfig.readEp);
      continue;
    }
    if (rc == LIBUSB_ERROR_TIMEOUT) {
      rc = LIBUSB_SUCCESS;
    }
    if (rc != LIBUSB_SUCCESS) break;
  }
  if (mDebug && timer.elapsed() >= mTimeout) qDebug("USB Timeout!");

  // we resize the buffer.
  mReadBuffer.resize(read_total);
  buf->append(mReadBuffer.mid(0, len));
  mReadBuffer.remove(0, len);

  QString datastr, s;

  if (mDebug) {
    for (qint32 i = 0; i < read_total; i++) {
      datastr.append(s.sprintf("%02X:", (uchar)buf->at(i)));
    }
    datastr.remove(datastr.size() - 1, 1);  // remove last colon
    qDebug("%d bytes Received: %s", read_total, datastr.toStdString().c_str());
  }

  if (rc != LIBUSB_SUCCESS) {
    this->printUsbError(rc);
    return rc;
  }

  return read_total;
}

qint32 QUsbDevice::write(const QByteArray* buf, quint32 len) {
  UsbPrintFuncName();
  Q_CHECK_PTR(buf);
  qint32 rc, sent_tmp;
  qint32 sent;
  QElapsedTimer timer;

  // check it isn't closed
  if (!mDevHandle || !mConnected) return -1;

  if (mDebug) {
    QString cmd, s;
    for (quint32 i = 0; i < len; i++) {
      cmd.append(s.sprintf("%02X:", (uchar)buf->at(i)));
    }
    cmd.remove(cmd.size() - 1, 1);  // remove last colon;
    qDebug() << "Sending" << len << "bytes:" << cmd;
    // qDebug("Sending %ll bytes: %s", maxSize, cmd.toStdString().c_str());
  }

  sent = 0;
  sent_tmp = 0;

  timer.start();
  while (timer.elapsed() < mTimeout && len - sent > 0) {
    rc = libusb_bulk_transfer(mDevHandle, (mConfig.writeEp),
                              (uchar*)buf->data(), len, &sent_tmp, mTimeout);
    if (rc == LIBUSB_ERROR_PIPE) {
      libusb_clear_halt(mDevHandle, mConfig.readEp);
    }
    sent += sent_tmp;
    if (rc != LIBUSB_SUCCESS) break;
  }

  if ((qint32)len != sent) {
    qWarning("Only sent %d out of %d", sent, len);
    return -1;
  }

  if (rc != LIBUSB_SUCCESS) {
    this->printUsbError(rc);
    return rc;
  }

  return sent;
}
