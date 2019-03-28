#include "qusbinfo.h"
#include "qusbinfo_p.h"
#include <QThread>

#define UsbPrintError() qWarning("In %s, at %s:%d", Q_FUNC_INFO, __FILE__, __LINE__)
#define UsbPrintFuncName() if (m_debug) qDebug() << "***[" << Q_FUNC_INFO << "]***"

static libusb_hotplug_callback_handle callback_handle;

static int LIBUSB_CALL hotplugCallback(libusb_context *ctx,
                                       libusb_device *device,
                                       libusb_hotplug_event event,
                                       void *user_data) {

  static libusb_device_handle *handle = Q_NULLPTR;
  struct libusb_device_descriptor desc;
  int rc;
  (void)ctx;
  QUsbDevice::FilterList device_list;
  QUsbDevice::DeviceFilter dev;
  QUsbInfo *manager = reinterpret_cast<QUsbInfo*>(user_data);

  if (manager->debug())
    qDebug("hotplugCallback");

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

QUsbInfoPrivate::QUsbInfoPrivate() : m_refresh_timer(new QTimer)
{
  QThread* t = new QThread();
  m_refresh_timer->moveToThread(t);

  m_refresh_timer->setSingleShot(false);
  m_refresh_timer->setInterval(250);

  m_refresh_timer->connect(t, SIGNAL(started()), SLOT(start()));
  t->start();
}

QUsbInfoPrivate::~QUsbInfoPrivate()
{
  m_refresh_timer->stop();
  m_refresh_timer->disconnect();
  m_refresh_timer->deleteLater();

  m_refresh_timer->thread()->exit();
  m_refresh_timer->thread()->wait();
  m_refresh_timer->thread()->deleteLater();
}

void QUsbInfo::checkDevices()
{
  UsbPrintFuncName();
  Q_D(QUsbInfo);
  QUsbDevice::FilterList list;

  timeval t = {0, 0};

  if (d->m_has_hotplug) {
    libusb_handle_events_timeout_completed(d->m_ctx, &t, Q_NULLPTR);
  } else {
    list = QUsbDevice::availableDevices();
    monitorDevices(list);
  }
}

QUsbInfo::QUsbInfo(QObject *parent) : QObject(*(new QUsbInfoPrivate), parent), d_dummy(Q_NULLPTR) {
  Q_D(QUsbInfo);

  m_debug = false;
  int rc;

  qRegisterMetaType<QUsbDevice::DeviceFilter>("QUsbDevice::DeviceFilter");
  qRegisterMetaType<QUsbDevice::DeviceConfig>("QUsbDevice::DeviceConfig");

  qRegisterMetaType<QUsbDevice::FilterList>("QUsbDevice::FilterList");
  qRegisterMetaType<QUsbDevice::ConfigList>("QUsbDevice::ConfigList");

  rc = libusb_init(&d->m_ctx);
  if (rc < 0) {
    libusb_exit(d->m_ctx);
    qCritical("LibUsb Init Error %d", rc);
    return;
  }

  libusb_set_debug(d->m_ctx, LIBUSB_LOG_LEVEL_WARNING);

  // Populate list once
  m_system_list = QUsbDevice::availableDevices();

  // Try hotplug first
  d->m_has_hotplug = libusb_has_capability(LIBUSB_CAP_HAS_HOTPLUG) != 0;
  if (d->m_has_hotplug) {

    rc = libusb_hotplug_register_callback(d->m_ctx,
                                          static_cast<libusb_hotplug_event>((LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT)),
                                          static_cast<libusb_hotplug_flag>(0),
                                          LIBUSB_HOTPLUG_MATCH_ANY,
                                          LIBUSB_HOTPLUG_MATCH_ANY,
                                          LIBUSB_HOTPLUG_MATCH_ANY,
                                          reinterpret_cast<libusb_hotplug_callback_fn>(hotplugCallback),
                                          reinterpret_cast<void*>(this),
                                          &callback_handle);
    if (LIBUSB_SUCCESS != rc) {
      libusb_exit(d->m_ctx);
      qWarning("Error creating hotplug callback");
      return;
    }
  }

  connect(d->m_refresh_timer, SIGNAL(timeout()), this, SLOT(checkDevices()));
}

QUsbInfo::~QUsbInfo() {
  Q_D(QUsbInfo);
  libusb_hotplug_deregister_callback(d->m_ctx, callback_handle);
  libusb_exit(d->m_ctx);
}

QUsbDevice::FilterList QUsbInfo::getPresentDevices() {
  UsbPrintFuncName();
  QUsbDevice::FilterList list;
  QUsbDevice::DeviceFilter filter;

  /* Search the system list with our own list */
  for (int i = 0; i < m_filter_list.length(); i++) {
    filter = m_filter_list.at(i);
    if (this->findDevice(filter, m_system_list) < 0) {
      list.append(filter);
    }
  }
  return list;
}

bool QUsbInfo::isPresent(const QUsbDevice::DeviceFilter &filter) {

  return this->findDevice(filter, m_system_list) >= 0;
}

bool QUsbInfo::addDevice(const QUsbDevice::DeviceFilter &filter) {

  if (this->findDevice(filter, m_filter_list) == -1) {
    m_filter_list.append(filter);
    return true;
  }
  return false;
}

bool QUsbInfo::removeDevice(const QUsbDevice::DeviceFilter &filter) {

  const int pos = this->findDevice(filter, m_filter_list);
  if (pos > 0) {
    m_filter_list.removeAt(pos);
    return true;
  }
  return true;
}

int QUsbInfo::findDevice(const QUsbDevice::DeviceFilter &filter,
                            const QUsbDevice::FilterList &list) {
  for (int i = 0; i < list.length(); i++) {
    const QUsbDevice::DeviceFilter *d = &list.at(i);

    if (d->pid == filter.pid && d->vid == filter.vid) {
      return i;
    }
  }
  return -1;
}

void QUsbInfo::setDebug(bool debug)
{
  UsbPrintFuncName();
  Q_D(QUsbInfo);
  m_debug = debug;
  if (m_debug)
    libusb_set_debug(d->m_ctx, LIBUSB_LOG_LEVEL_DEBUG);
  else
    libusb_set_debug(d->m_ctx, LIBUSB_LOG_LEVEL_WARNING);
}

QUsbDevice::DeviceStatus QUsbInfo::openDevice(QUsbDevice *dev,
                                            const QUsbDevice::DeviceFilter &filter,
                                            const QUsbDevice::DeviceConfig &config) {
  UsbPrintFuncName();
  if (dev == Q_NULLPTR)
    return QUsbDevice::statusNoSuchDevice;
  dev->setConfig(config);
  dev->setFilter(filter);

  m_used_device_list.append(dev);
  if (dev->open() == 0)
    return QUsbDevice::statusOK;
  else
    return QUsbDevice::statusNotFound;
}

QUsbDevice::DeviceStatus QUsbInfo::closeDevice(QUsbDevice *dev) {

  UsbPrintFuncName();
  if (dev != Q_NULLPTR) {
    int pos = m_used_device_list.indexOf(dev);
    m_used_device_list.removeAt(pos);
    dev->close();
    return QUsbDevice::statusOK;
  }
  return QUsbDevice::statusNoSuchDevice;
}

void QUsbInfo::monitorDevices(const QUsbDevice::FilterList &list) {

  UsbPrintFuncName();
  QUsbDevice::FilterList inserted, removed;
  QUsbDevice::DeviceFilter filter;

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
