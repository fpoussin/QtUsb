#include "qusbinfo.h"
#include "qusbinfo_p.h"
#include <QThread>

#define DbgPrintError() qWarning("In %s, at %s:%d", Q_FUNC_INFO, __FILE__, __LINE__)
#define DbgPrintFuncName()                   \
    if (m_log_level >= QUsbDevice::logDebug) \
    qDebug() << "***[" << Q_FUNC_INFO << "]***"
#define DbgPrintCB()                              \
    if (info->logLevel() >= QUsbDevice::logDebug) \
    qDebug() << "***[" << Q_FUNC_INFO << "]***"

static libusb_hotplug_callback_handle callback_handle;

static int LIBUSB_CALL hotplugCallback(libusb_context *ctx,
                                       libusb_device *device,
                                       libusb_hotplug_event event,
                                       void *user_data)
{
    struct libusb_device_descriptor desc;
    (void)ctx;
    QUsbDevice::IdList device_list;
    QUsbDevice::Id id;
    QUsbInfo *info = reinterpret_cast<QUsbInfo *>(user_data);
    DbgPrintCB();

    uint8_t bus = libusb_get_bus_number(device);
    uint8_t port = libusb_get_port_number(device);

    if (info->logLevel() >= QUsbDevice::logDebug)
        qDebug("hotplugCallback");

    libusb_get_device_descriptor(device, &desc);
    id.vid = desc.idVendor;
    id.pid = desc.idProduct;
    id.bus = bus;
    id.port = port;
    id.dClass = desc.bDeviceClass;
    id.dSubClass = desc.bDeviceSubClass;

    if (event == LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED) {

        // Add to list
        emit info->deviceInserted(id);

    } else if (event == LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT) {

        // Remove from list
        emit info->deviceRemoved(id);

    } else {
        if (info->logLevel() >= QUsbDevice::logWarning)
            qWarning("Unhandled hotplug event %d", event);
        return -1;
    }
    return 0;
}

QUsbInfoPrivate::QUsbInfoPrivate()
    : m_has_hotplug(false), m_ctx(Q_NULLPTR), m_refresh_timer(new QTimer)
{
    QThread *t = new QThread();
    m_refresh_timer->moveToThread(t);

    m_refresh_timer->setSingleShot(false);
    m_refresh_timer->setInterval(250);

    m_refresh_timer->connect(t, SIGNAL(started()), SLOT(start()));
    m_refresh_timer->connect(t, SIGNAL(finished()), SLOT(stop()));
    t->start();
}

QUsbInfoPrivate::~QUsbInfoPrivate()
{
    m_refresh_timer->thread()->exit();
    m_refresh_timer->thread()->wait();
    m_refresh_timer->thread()->deleteLater();

    m_refresh_timer->disconnect();
    m_refresh_timer->deleteLater();
}

/*!
    \class QUsbInfo

    \brief This class handles hotplug and device detection.

    \reentrant
    \ingroup usb-main
    \inmodule QtUsb

    Handles USB events and searching.
    Can be used to monitor events for a list of devices or all system devices.

    \sa QUsbDevice
*/

/*!
    \fn void QUsbInfo::deviceInserted(QUsbDevice::IdList list)

    This is signal is emited when one or more new devices are detected, providing a \a list.
*/

/*!
    \fn void QUsbInfo::deviceRemoved(QUsbDevice::IdList list)

    This is signal is emited when one or more new devices are removed, providing a \a list.
*/

/*!
    \property QUsbInfo::logLevel
    \brief the log level for hotplug/detection.
*/

QUsbInfo::QUsbInfo(QObject *parent)
    : QObject(*(new QUsbInfoPrivate), parent), d_dummy(Q_NULLPTR)
{
    Q_D(QUsbInfo);

    m_log_level = QUsbDevice::logInfo;
    DbgPrintFuncName();
    int rc;

    qRegisterMetaType<QUsbDevice::Id>("QUsbDevice::DeviceFilter");
    qRegisterMetaType<QUsbDevice::Config>("QUsbDevice::DeviceConfig");

    qRegisterMetaType<QUsbDevice::IdList>("QUsbDevice::FilterList");
    qRegisterMetaType<QUsbDevice::ConfigList>("QUsbDevice::ConfigList");

    rc = libusb_init(&d->m_ctx);
    if (rc < 0) {
        libusb_exit(d->m_ctx);
        qCritical("LibUsb Init Error %d", rc);
        return;
    }

    libusb_set_debug(d->m_ctx, LIBUSB_LOG_LEVEL_WARNING);

    // Populate list once
    m_system_list = devices();

    // Try hotplug first
    d->m_has_hotplug = libusb_has_capability(LIBUSB_CAP_HAS_HOTPLUG) != 0;
    if (d->m_has_hotplug) {

        rc = libusb_hotplug_register_callback(d->m_ctx,
                                              static_cast<libusb_hotplug_event>((LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT)),
                                              LIBUSB_HOTPLUG_NO_FLAGS,
                                              LIBUSB_HOTPLUG_MATCH_ANY,
                                              LIBUSB_HOTPLUG_MATCH_ANY,
                                              LIBUSB_HOTPLUG_MATCH_ANY,
                                              reinterpret_cast<libusb_hotplug_callback_fn>(hotplugCallback),
                                              reinterpret_cast<void *>(this),
                                              &callback_handle);
        if (LIBUSB_SUCCESS != rc) {
            libusb_exit(d->m_ctx);
            qWarning("Error creating hotplug callback");
            return;
        }
    }

    connect(d->m_refresh_timer, SIGNAL(timeout()), this, SLOT(checkDevices()));
}

/*!
    Unregister callbacks and close the usb context.
 */
QUsbInfo::~QUsbInfo()
{
    Q_D(QUsbInfo);
    DbgPrintFuncName();
    libusb_hotplug_deregister_callback(d->m_ctx, callback_handle);
    libusb_exit(d->m_ctx);
}

/*!
    Check devices present in system.

    This gets called by the internal timer.
 */
void QUsbInfo::checkDevices()
{
    DbgPrintFuncName();
    Q_D(QUsbInfo);
    QUsbDevice::IdList list;

    timeval t = { 0, 0 };

    if (d->m_has_hotplug) {
        libusb_handle_events_timeout_completed(d->m_ctx, &t, Q_NULLPTR);
    } else {
        list = devices();
        monitorDevices(list);
    }
}

/*!
    \brief Returns all present \c devices.
 */
QUsbDevice::IdList QUsbInfo::devices()
{
    QUsbDevice::IdList list;
    ssize_t cnt; // holding number of devices in list
    libusb_device **devs;
    libusb_context *ctx;

    libusb_init(&ctx);
    libusb_set_debug(ctx, LIBUSB_LOG_LEVEL_NONE);
    cnt = libusb_get_device_list(ctx, &devs); // get the list of devices
    if (cnt < 0) {
        qCritical("libusb_get_device_list Error");
        libusb_free_device_list(devs, 1);
        return list;
    }

    for (int i = 0; i < cnt; i++) {
        libusb_device *dev = devs[i];
        libusb_device_descriptor desc;

        if (libusb_get_device_descriptor(dev, &desc) == 0) {
            QUsbDevice::Id id;
            id.pid = desc.idProduct;
            id.vid = desc.idVendor;
            id.bus = libusb_get_bus_number(dev);
            id.port = libusb_get_port_number(dev);

            list.append(id);
        }
    }

    libusb_free_device_list(devs, 1);
    libusb_exit(ctx);
    return list;
}

/*!
      Check if \a id  device is present.

      Return bool true if present.
 */
bool QUsbInfo::isPresent(const QUsbDevice::Id &id) const
{
    DbgPrintFuncName();
    return this->findDevice(id, m_system_list) >= 0;
}

/*!
      Add an \a id device to the list.

      Returns false if device was already in the list, else true.
 */
bool QUsbInfo::addDevice(const QUsbDevice::Id &id)
{

    DbgPrintFuncName();
    if (this->findDevice(id, m_list) == -1) {
        m_list.append(id);
        return true;
    }
    return false;
}

/*!
      \brief Remove \a id device from the list.

      Return bool false if device was not in the list, else true.
 */
bool QUsbInfo::removeDevice(const QUsbDevice::Id &id)
{

    DbgPrintFuncName();
    const int pos = this->findDevice(id, m_list);
    if (pos > 0) {
        m_list.removeAt(pos);
        return true;
    }
    return true;
}

/*!
      Search an \a id device in a device \a list.

      Return index of the filter, returns -1 if not found.
 */
int QUsbInfo::findDevice(const QUsbDevice::Id &id,
                         const QUsbDevice::IdList &list) const
{
    DbgPrintFuncName();
    for (int i = 0; i < list.length(); i++) {
        const QUsbDevice::Id *d = &list.at(i);

        if (d->pid == id.pid && d->vid == id.vid) {
            if (id.bus == QUsbDevice::busAny && id.port == QUsbDevice::portAny) // Ignore bus/port if both == any
                return i;
            if (d->bus == id.bus && d->port == id.port) // Take bus/port into account for filtering when set
                return i;
        }
    }
    return -1;
}

/*!
    Set log \a level (only hotplug/detection).
 */
void QUsbInfo::setLogLevel(QUsbDevice::LogLevel level)
{
    DbgPrintFuncName();
    Q_D(QUsbInfo);
    m_log_level = level;
    if (m_log_level >= QUsbDevice::logDebug)
        libusb_set_debug(d->m_ctx, LIBUSB_LOG_LEVEL_DEBUG);
    else if (m_log_level >= QUsbDevice::logWarning)
        libusb_set_debug(d->m_ctx, LIBUSB_LOG_LEVEL_WARNING);
    else
        libusb_set_debug(d->m_ctx, LIBUSB_LOG_LEVEL_ERROR);
}

/*!
    Get current log level.
 */
QUsbDevice::LogLevel QUsbInfo::logLevel() const
{
    return m_log_level;
}

/*!
    Add a \a list to monitor.
 */
void QUsbInfo::monitorDevices(const QUsbDevice::IdList &list)
{

    DbgPrintFuncName();
    QUsbDevice::IdList inserted, removed;
    QUsbDevice::Id filter;

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

    for (int i = 0; i < inserted.length(); i++) {
        emit deviceInserted(inserted.at(i));
    }

    for (int i = 0; i < removed.length(); i++) {
        emit deviceRemoved(removed.at(i));
    }

    m_system_list = list;
}
