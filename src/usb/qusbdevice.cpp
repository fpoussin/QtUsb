#include "qusbdevice.h"
#include "qusbdevice_p.h"
#include <QElapsedTimer>

#define UsbPrintError() qWarning("In %s, at %s:%d", Q_FUNC_INFO, __FILE__, __LINE__)
#define UsbPrintFuncName() if (m_debug) qDebug() << "***[" << Q_FUNC_INFO << "]***"

QUsbDevicePrivate::QUsbDevicePrivate()
{

}

void QUsbDevicePrivate::setDefaults() {
    Q_Q(QUsbDevice);
    q->m_connected = false;
    q->m_debug = false;
    q->m_timeout = QtUsb::DefaultTimeout;
    q->m_config.config = 0x01;
    q->m_config.interface = 0x00;
    q->m_config.alternate = 0x00;
}

void QUsbDevicePrivate::printUsbError(int error_code) const
{
    qWarning("libusb Error: %s", libusb_strerror(static_cast<enum libusb_error>(error_code)));
}

QUsbDevice::QUsbDevice(QObject* parent) : QObject(*(new QUsbDevicePrivate), parent), d_dummy(Q_NULLPTR) {
  Q_D(QUsbDevice);
  d->m_devHandle = Q_NULLPTR;
  d->setDefaults();
  m_spd = QtUsb::unknownSpeed;
  int r = libusb_init(
      &d->m_ctx);  // initialize the library for the session we just declared
  if (r < 0) {
    qCritical("LibUsb Init Error %d", r);  // there was an error
  }
  m_read_buffer_size = 1024 * 64;
}

QUsbDevice::~QUsbDevice() {
  Q_D(QUsbDevice);
  this->close();
  libusb_exit(d->m_ctx);
}


QByteArray QUsbDevice::speedString() const {
    switch (m_spd) {
    case QtUsb::unknownSpeed:
        return "Unknown speed";
    case QtUsb::lowSpeed:
        return "Low speed";
    case QtUsb::fullSpeed:
        return "Full speed";
    case QtUsb::highSpeed:
        return "High speed";
    case QtUsb::superSpeed:
        return "Super speed";
    }

    return "Error";
}

qint32 QUsbDevice::write(const QByteArray &buf, QtUsb::endpoint endpoint) {
    return this->write(&buf, buf.size(), endpoint);
}

qint32 QUsbDevice::read(QByteArray *buf, QtUsb::endpoint endpoint) { return this->read(buf, 4096, endpoint); }

bool QUsbDevice::write(char c, QtUsb::endpoint endpoint) {
    QByteArray buf(1, c);
    return this->write(buf, endpoint) > 0;
}

bool QUsbDevice::read(char *c, QtUsb::endpoint endpoint) {
    QByteArray buf;
    Q_CHECK_PTR(c);
    if (this->read(&buf, 1, endpoint) > 0) {
        *c = buf.at(0);
        return true;
    }
    return false;
}

void QUsbDevice::showSettings() {
    qWarning() << "\n"
               << "Debug" << m_debug << "\n"
               << "Config" << m_config.config << "\n"
               << "Timeout" << m_timeout << "\n"
               << "Interface" << m_config.interface << "\n"
               << "Device.pid" << QString::number(m_filter.pid, 16) << "\n"
               << "Device.vid" << QString::number(m_filter.vid, 16) << "\n";
}

QtUsb::FilterList QUsbDevice::availableDevices() {
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
  Q_D(QUsbDevice);

  int rc = -5;   // Not found by default
  ssize_t cnt;  // holding number of devices in list
  libusb_device* dev = Q_NULLPTR;

  if (m_connected) return -1;

  cnt = libusb_get_device_list(d->m_ctx, &d->m_devs);  // get the list of devices
  if (cnt < 0) {
    qCritical("libusb_get_device_list error");
    libusb_free_device_list(d->m_devs, 1);
    return -1;
  }

  for (int i = 0; i < cnt; i++) {
    dev = d->m_devs[i];
    libusb_device_descriptor desc;

    if (libusb_get_device_descriptor(dev, &desc) == 0) {
      if (desc.idProduct == m_filter.pid && desc.idVendor == m_filter.vid) {
        if (m_debug) {
          qDebug("Found device.");
        }
        rc = libusb_open(dev, &d->m_devHandle);
        if (rc == 0) break;
        else {
          qWarning("Failed to open device: %s", libusb_strerror(static_cast<enum libusb_error>(rc)));
        }
      }
    }
  }
  libusb_free_device_list(d->m_devs, 1);  // free the list, unref the devices in it

  if (rc != 0 || d->m_devHandle == Q_NULLPTR) {
    return rc;
  }

  if (m_debug) qDebug("Device Opened");

  if (libusb_kernel_driver_active(d->m_devHandle, m_config.interface) ==
      1) {  // find out if kernel driver is attached
    if (m_debug) qDebug("Kernel Driver Active");
    if (libusb_detach_kernel_driver(d->m_devHandle, m_config.interface) ==
        0)  // detach it
      if (m_debug) qDebug("Kernel Driver Detached!");
  }

  int conf;
  libusb_get_configuration(d->m_devHandle, &conf);

  if (conf != m_config.config) {
    if (m_debug) qDebug("Configuration needs to be changed");
    rc = libusb_set_configuration(d->m_devHandle, m_config.config);
    if (rc != 0) {
      qWarning("Cannot Set Configuration");
      d->printUsbError(rc);
      return -3;
    }
  }
  rc = libusb_claim_interface(d->m_devHandle, m_config.interface);
  if (rc != 0) {
    qWarning("Cannot Claim Interface");
    d->printUsbError(rc);
    return -4;
  }

  switch (libusb_get_device_speed(dev)) {
    case LIBUSB_SPEED_LOW:
      this->m_spd = QtUsb::lowSpeed;
      break;
    case LIBUSB_SPEED_FULL:
      this->m_spd = QtUsb::fullSpeed;
      break;
    case LIBUSB_SPEED_HIGH:
      this->m_spd = QtUsb::highSpeed;
      break;
    case LIBUSB_SPEED_SUPER:
      this->m_spd = QtUsb::superSpeed;
      break;
    default:
      this->m_spd = QtUsb::unknownSpeed;
      break;
  }

  m_connected = true;

  return 0;
}

void QUsbDevice::close() {
  UsbPrintFuncName();
  Q_D(QUsbDevice);

  if (d->m_devHandle && m_connected) {
    // stop any further write attempts whilst we close down
    qDebug("Closing USB connection...");

    QUsbDevice::close();

    libusb_release_interface(d->m_devHandle, 0);  // release the claimed interface
    libusb_close(d->m_devHandle);                 // close the device we opened
  }

  m_connected = false;
}

void QUsbDevice::setDebug(bool enable) {
  Q_D(QUsbDevice);
  m_debug = enable;
  if (enable)
    libusb_set_debug(d->m_ctx, LIBUSB_LOG_LEVEL_DEBUG);
  else
    libusb_set_debug(d->m_ctx, LIBUSB_LOG_LEVEL_ERROR);
}

void QUsbDevice::flush(quint8 endpoint) {
  Q_D(QUsbDevice);
  QByteArray buf;
  qint32 read_bytes;

  buf.resize(4096);
  libusb_bulk_transfer(d->m_devHandle,
                       endpoint,
                       reinterpret_cast<uchar*>(buf.data()),
                       4096,
                       &read_bytes, 25);
}

qint32 QUsbDevice::read(QByteArray* buf, int len, QtUsb::endpoint endpoint) {
  UsbPrintFuncName();
  Q_D(QUsbDevice);
  Q_CHECK_PTR(buf);
  qint32 rc, read_bytes;
  qint32 read_total;
  QElapsedTimer timer;

  // check it isn't closed already
  if (!d->m_devHandle || !m_connected) return -1;

  if (len == 0) return 0;

  read_total = 0;
  read_bytes = 0;

  if (m_read_buffer.isEmpty())
    if (m_debug) qDebug("Read cache empty");

  /* Fetch from buffer first */
  if (len <= m_read_buffer.size() && !m_read_buffer.isEmpty()) {
    if (m_debug) qDebug("Reading %d bytes from cache", m_read_buffer.size());
    *buf = m_read_buffer.mid(0, (len));
    m_read_buffer.remove(0, len);
    return static_cast<int>(len);
  }

  /* Copy what's in the read buffer */
  len -= static_cast<quint32>(m_read_buffer.size());
  *buf = m_read_buffer;
  m_read_buffer.clear();
  m_read_buffer.resize(m_read_buffer_size);

  /* Wait till we have at least the required data */
  timer.start();
  rc = LIBUSB_SUCCESS;
  while (timer.elapsed() < m_timeout && len - read_total > 0) {
    rc = libusb_bulk_transfer(d->m_devHandle,
                              endpoint,
                              reinterpret_cast<uchar*>((m_read_buffer.data() + read_total)),
                              m_read_buffer_size,
                              &read_bytes,
                              10);
    read_total += read_bytes;
    if (rc == LIBUSB_ERROR_PIPE) {
      libusb_clear_halt(d->m_devHandle, endpoint);
      continue;
    }
    if (rc == LIBUSB_ERROR_TIMEOUT) {
      rc = LIBUSB_SUCCESS;
    }
    if (rc != LIBUSB_SUCCESS) break;
  }
  if (m_debug && timer.elapsed() >= m_timeout) qDebug("USB Timeout!");

  // we resize the buffer.
  m_read_buffer.resize(read_total);
  buf->append(m_read_buffer.mid(0, len));
  m_read_buffer.remove(0, len);

  QString datastr, s;

  if (m_debug) {
    for (int i = 0; i < read_total; i++) {
      datastr.append(s.sprintf("%02X:", buf->at(i)));
    }
    datastr.remove(datastr.size() - 1, 1);  // remove last colon
    qDebug("%d bytes Received: %s", read_total, datastr.toStdString().c_str());
  }

  if (rc != LIBUSB_SUCCESS) {
    d->printUsbError(rc);
    return rc;
  }

  return read_total;
}

qint32 QUsbDevice::write(const QByteArray* buf, int len, QtUsb::endpoint endpoint) {
  UsbPrintFuncName();
  Q_D(QUsbDevice);
  Q_CHECK_PTR(buf);
  qint32 rc, sent_tmp;
  qint32 sent;
  QElapsedTimer timer;

  // check it isn't closed
  if (!d->m_devHandle || !m_connected) return -1;

  if (m_debug) {
    QString cmd, s;
    for (int i = 0; i < len; i++) {
      cmd.append(s.sprintf("%02X:", buf->at(i)));
    }
    cmd.remove(cmd.size() - 1, 1);  // remove last colon;
    qDebug() << "Sending" << len << "bytes:" << cmd;
    // qDebug("Sending %ll bytes: %s", maxSize, cmd.toStdString().c_str());
  }

  sent = 0;
  sent_tmp = 0;
  rc = -99;

  timer.start();
  while (timer.elapsed() < m_timeout && len - sent > 0) {
    QByteArray b(buf->mid(0, len)); // Need a local copy since we got const data
    rc = libusb_bulk_transfer(d->m_devHandle,
                              endpoint,
                              reinterpret_cast<unsigned char*>(b.data()),
                              len,
                              &sent_tmp,
                              m_timeout);
    if (rc == LIBUSB_ERROR_PIPE) {
      libusb_clear_halt(d->m_devHandle, endpoint);
    }
    sent += sent_tmp;
    if (rc != LIBUSB_SUCCESS) break;
  }

  if (len != sent) {
    qWarning("Only sent %d out of %d", sent, len);
    return -1;
  }

  if (rc != LIBUSB_SUCCESS) {
    d->printUsbError(rc);
    return rc;
  }

  return sent;
}
