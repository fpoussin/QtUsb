#include "qusbmanager.h"
#include "qusbmanager_p.h"

static libusb_hotplug_callback_handle callback_handle;

static int hotplugCallback(libusb_context *ctx,
                           libusb_device *device,
                           libusb_hotplug_event event,
                           void *user_data) {

  qDebug("hotplugCallback");

  static libusb_device_handle *handle = Q_NULLPTR;
  struct libusb_device_descriptor desc;
  int rc;
  (void)ctx;
  QtUsb::FilterList device_list;
  QtUsb::DeviceFilter dev;
  QUsbManager *manager = static_cast<QUsbManager*>(user_data);

  (void)libusb_get_device_descriptor(device, &desc);
  if (event == LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED) {
    rc = libusb_open(device, &handle);
    if (LIBUSB_SUCCESS != rc) {
      qWarning("Could not open new USB device");
      return -1;
    }
    // Add to list
    dev.vid = desc.idVendor;
    dev.pid = desc.idProduct;
    device_list.append(dev);
    emit manager->deviceInserted(device_list);

  } else if (event == LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT) {
    if (handle) {
      // Remove from list
      dev.vid = desc.idVendor;
      dev.pid = desc.idProduct;
      device_list.append(dev);
      emit manager->deviceRemoved(device_list);

      libusb_close(handle);
      handle = Q_NULLPTR;
      return 0;
    }
  } else {
    qWarning("Unhandled hotplug event %d", event);
    return -1;
  }
  return 0;
}

QUsbManager::QUsbManager(QObject *parent) : QThread(parent) {
  Q_D(QUsbManager);

  m_stop = false;
  int rc;

  qRegisterMetaType<QtUsb::DeviceFilter>("QtUsb::DeviceFilter");
  qRegisterMetaType<QtUsb::DeviceConfig>("QtUsb::DeviceConfig");

  qRegisterMetaType<QtUsb::FilterList>("QtUsb::FilterList");
  qRegisterMetaType<QtUsb::ConfigList>("QtUsb::ConfigList");

  rc = libusb_init(&d->m_ctx);
  if (rc < 0) {
    libusb_exit(d->m_ctx);
    qCritical("LibUsb Init Error %d", rc);
    return;
  }

  libusb_set_debug(d->m_ctx, LIBUSB_LOG_LEVEL_DEBUG);

  // Populate list once
  m_system_list = QUsbDevice::availableDevices();

  // Try hotplug first
  m_has_hotplug = libusb_has_capability(LIBUSB_CAP_HAS_HOTPLUG) != 0;
  if (m_has_hotplug) {
    rc = libusb_hotplug_register_callback(d->m_ctx,
                                          static_cast<libusb_hotplug_event>((LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT)),
                                          static_cast<libusb_hotplug_flag>(0),
                                          LIBUSB_HOTPLUG_MATCH_ANY,
                                          LIBUSB_HOTPLUG_MATCH_ANY,
                                          LIBUSB_HOTPLUG_MATCH_ANY,
                                          static_cast<libusb_hotplug_callback_fn>(hotplugCallback),
                                          static_cast<void*>(this),
                                          &callback_handle);
    if (LIBUSB_SUCCESS != rc) {
      libusb_exit(d->m_ctx);
      qWarning("Error creating hotplug callback");
      return;
    }
  }

  connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
  this->start();
}

QUsbManager::~QUsbManager() {

  m_stop = true;
  this->wait();
}

QtUsb::FilterList QUsbManager::getPresentDevices() {

  QtUsb::FilterList list;
  QtUsb::DeviceFilter filter;

  /* Search the system list with our own list */
  for (int i = 0; i < m_filter_list.length(); i++) {
    filter = m_filter_list.at(i);
    if (this->findDevice(filter, m_system_list) < 0) {
      list.append(filter);
    }
  }
  return list;
}

bool QUsbManager::isPresent(const QtUsb::DeviceFilter &filter) {

  return this->findDevice(filter, m_system_list) >= 0;
}

bool QUsbManager::addDevice(const QtUsb::DeviceFilter &filter) {

  if (this->findDevice(filter, m_filter_list) == -1) {
    m_filter_list.append(filter);
    return true;
  }
  return false;
}

bool QUsbManager::removeDevice(const QtUsb::DeviceFilter &filter) {

  const int pos = this->findDevice(filter, m_filter_list);
  if (pos > 0) {
    m_filter_list.removeAt(pos);
    return true;
  }
  return true;
}

int QUsbManager::findDevice(const QtUsb::DeviceFilter &filter,
                            const QtUsb::FilterList &list) {
  for (int i = 0; i < list.length(); i++) {
    const QtUsb::DeviceFilter *d = &list.at(i);

    if (d->pid == filter.pid && d->vid == filter.vid) {
      return i;
    }
  }
  return -1;
}

void QUsbManager::setDebug(bool debug)
{
  Q_D(QUsbManager);
  m_debug = debug;
  if (m_debug)
    libusb_set_debug(d->m_ctx, LIBUSB_LOG_LEVEL_DEBUG);
  else
    libusb_set_debug(d->m_ctx, LIBUSB_LOG_LEVEL_WARNING);
}

QtUsb::DeviceStatus QUsbManager::openDevice(QUsbDevice *dev,
                                            const QtUsb::DeviceFilter &filter,
                                            const QtUsb::DeviceConfig &config) {

  if (dev == Q_NULLPTR)
    return QtUsb::devicePgmError;
  dev->setConfig(config);
  dev->setFilter(filter);

  m_used_device_list.append(dev);
  if (dev->open() == 0)
    return QtUsb::deviceOK;
  else
    return QtUsb::deviceNotFound;
}

QtUsb::DeviceStatus QUsbManager::closeDevice(QUsbDevice *dev) {

  if (dev != Q_NULLPTR) {
    int pos = m_used_device_list.indexOf(dev);
    m_used_device_list.removeAt(pos);
    dev->close();
    return QtUsb::deviceOK;
  }
  return QtUsb::devicePgmError;
}

void QUsbManager::monitorDevices(const QtUsb::FilterList &list) {

  QtUsb::FilterList inserted, removed;
  QtUsb::DeviceFilter filter;

  for (int i = 0; i < list.length(); i++) {
    filter = list.at(i);
    if (this->findDevice(filter, m_system_list) < 0) {
      // It's not in the old system list
      inserted.append(filter);
    }
  }

  for (int i = 0; i < m_system_list.length(); i++) {
    filter = m_system_list.at(i);
    if (this->findDevice(filter, list) < 0) {
      // It's in the old system list but not in the current one
      removed.append(filter);
    }
  }

  if (inserted.length() > 0)
    emit deviceInserted(inserted);

  if (removed.length() > 0)
    emit deviceRemoved(removed);

  m_system_list = list;
}

void QUsbManager::run() {

  Q_D(QUsbManager);
  QtUsb::FilterList list;
  while (!m_stop) {
    if (m_has_hotplug) {
      libusb_handle_events_completed(d->m_ctx, Q_NULLPTR);
    } else {
      list = QUsbDevice::availableDevices();
      this->monitorDevices(list);
    }
    this->msleep(250);
  }
}
