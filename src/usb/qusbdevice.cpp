#include "qusbdevice.h"
#include "qusbdevice_p.h"
#include <QElapsedTimer>

#define UsbPrintError() qWarning("In %s, at %s:%d", Q_FUNC_INFO, __FILE__, __LINE__)
#define UsbPrintFuncName() if (m_debug) qDebug() << "***[" << Q_FUNC_INFO << "]***"

QUsbDevicePrivate::QUsbDevicePrivate()
{
  int r = libusb_init(&m_ctx);
  if (r < 0) {
    qCritical("LibUsb Init Error %d", r);
  }
  m_devHandle = Q_NULLPTR;

  m_events = new QUsbEventsThread();
  m_events->m_ctx = m_ctx;
  m_events->start();
}

QUsbDevicePrivate::~QUsbDevicePrivate()
{
  Q_Q(QUsbDevice);
  m_events->requestInterruption();
  m_events->wait();
  m_events->deleteLater();

  q->close();
  libusb_exit(m_ctx);
}

void QUsbDevicePrivate::printUsbError(int error_code) const
{
  qWarning("libusb Error: %s", libusb_strerror(static_cast<enum libusb_error>(error_code)));
}

QUsbDevice::QUsbDevice(QObject* parent) : QObject(*(new QUsbDevicePrivate), parent), d_dummy(Q_NULLPTR) {
  m_spd = QtUsb::unknownSpeed;

  m_connected = false;
  m_debug = false;
  m_timeout = QtUsb::DefaultTimeout;
  m_config.config = 0x01;
  m_config.interface = 0x00;
  m_config.alternate = 0x00;
}

QUsbDevice::~QUsbDevice() {

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

    libusb_release_interface(d->m_devHandle, 0);  // release the claimed interface
    libusb_close(d->m_devHandle);                 // close the device we opened
    d->m_devHandle = Q_NULLPTR;
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

void QUsbEventsThread::run() {

  timeval t = {0, 100000};
  while (!this->isInterruptionRequested()) {
    if (libusb_handle_events_timeout_completed(m_ctx, &t, Q_NULLPTR) != 0) {
      break;
    }
  }
}
