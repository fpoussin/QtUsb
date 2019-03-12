#include "qusbmanager.h"

static libusb_hotplug_callback_handle callback_handle;
static int hotplugCallback(libusb_context *ctx,
                           libusb_device *device,
                           libusb_hotplug_event event,
                           void *user_data) {

  static libusb_device_handle *handle = NULL;
  struct libusb_device_descriptor desc;
  int rc;
  (void)ctx;
  QtUsb::FilterList device_list;
  QtUsb::DeviceFilter dev;
  QUsbManager *manager = (QUsbManager *)user_data;

  (void)libusb_get_device_descriptor(device, &desc);
  if (LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED == event) {
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

  } else if (LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT == event) {
    if (handle) {
      // Remove from list
      dev.vid = desc.idVendor;
      dev.pid = desc.idProduct;
      device_list.append(dev);
      emit manager->deviceRemoved(device_list);

      libusb_close(handle);
      handle = NULL;
      return 0;
    }
  } else {
    qWarning("Unhandled hotplug event %d", event);
    return -1;
  }
  return 0;
}

QUsbManager::QUsbManager(QObject *parent) : QThread(parent) {

  mStop = false;
  int rc;

  qRegisterMetaType<QtUsb::DeviceFilter>("QtUsb::DeviceFilter");
  qRegisterMetaType<QtUsb::DeviceConfig>("QtUsb::DeviceConfig");

  qRegisterMetaType<QtUsb::FilterList>("QtUsb::FilterList");
  qRegisterMetaType<QtUsb::ConfigList>("QtUsb::ConfigList");

  rc = libusb_init(&mCtx);
  if (rc < 0) {
    libusb_exit(mCtx);
    qCritical("LibUsb Init Error %d", rc);
    return;
  }

  // Populate list once
  mSystemList = QUsbDevice::getAvailableDevices();

  // Try hotplug first
  mHasHotplug = libusb_has_capability(LIBUSB_CAP_HAS_HOTPLUG) != 0;
  if (mHasHotplug) {
    rc = libusb_hotplug_register_callback(mCtx,
                                          (libusb_hotplug_event)(LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT),
                                          (libusb_hotplug_flag)0,
                                          LIBUSB_HOTPLUG_MATCH_ANY,
                                          LIBUSB_HOTPLUG_MATCH_ANY,
                                          LIBUSB_HOTPLUG_MATCH_ANY,
                                          (libusb_hotplug_callback_fn)hotplugCallback,
                                          (void*)this,
                                          &callback_handle);
    if (LIBUSB_SUCCESS != rc) {
      libusb_exit(mCtx);
      qWarning("Error creating hotplug callback");
      return;
    }
  }

  connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
  this->start();
}

QUsbManager::~QUsbManager() {

  mStop = true;
  this->wait();
}

QtUsb::FilterList QUsbManager::getPresentDevices() {

  QtUsb::FilterList list;
  QtUsb::DeviceFilter filter;

  /* Search the system list with our own list */
  for (int i = 0; i < mFilterList.length(); i++) {
    filter = mFilterList.at(i);
    if (this->findDevice(filter, mSystemList) < 0) {
      list.append(filter);
    }
  }
  return list;
}

bool QUsbManager::isPresent(const QtUsb::DeviceFilter &filter) {

  return this->findDevice(filter, mSystemList) >= 0;
}

bool QUsbManager::addDevice(const QtUsb::DeviceFilter &filter) {

  if (this->findDevice(filter, mFilterList) == -1) {
    mFilterList.append(filter);
    return true;
  }
  return false;
}

bool QUsbManager::removeDevice(const QtUsb::DeviceFilter &filter) {

  const int pos = this->findDevice(filter, mFilterList);
  if (pos > 0) {
    mFilterList.removeAt(pos);
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

QtUsb::DeviceStatus QUsbManager::openDevice(QUsbDevice *dev,
                                            const QtUsb::DeviceFilter &filter,
                                            const QtUsb::DeviceConfig &config) {

  if (dev == NULL)
    return QtUsb::devicePgmError;
  dev->setConfig(config);
  dev->setFilter(filter);

  mUsedDeviceList.append(dev);
  if (dev->open() == 0)
    return QtUsb::deviceOK;
  else
    return QtUsb::deviceNotFound;
}

QtUsb::DeviceStatus QUsbManager::closeDevice(QUsbDevice *dev) {

  if (dev != NULL) {
    int pos = mUsedDeviceList.indexOf(dev);
    mUsedDeviceList.removeAt(pos);
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
    if (this->findDevice(filter, mSystemList) < 0) {
      // It's not in the old system list
      inserted.append(filter);
    }
  }

  for (int i = 0; i < mSystemList.length(); i++) {
    filter = mSystemList.at(i);
    if (this->findDevice(filter, list) < 0) {
      // It's in the old system list but not in the current one
      removed.append(filter);
    }
  }

  if (inserted.length() > 0)
    emit deviceInserted(inserted);

  if (removed.length() > 0)
    emit deviceRemoved(removed);

  mSystemList = list;
}

void QUsbManager::run() {

  QtUsb::FilterList list;
  while (!mStop) {
    if (mHasHotplug) {
      libusb_handle_events_completed(mCtx, NULL);
    } else {
      list = QUsbDevice::getAvailableDevices();
      this->monitorDevices(list);
    }
    this->msleep(250);
  }
}
