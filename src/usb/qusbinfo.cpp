#include "qusbinfo.h"
#include "qusbinfo_p.h"
#include <QThread>

#define DbgPrintError() qWarning("In %s, at %s:%d", Q_FUNC_INFO, __FILE__, __LINE__)
#define DbgPrintFuncName()                 \
    if (m_log_level >= QUsbInfo::logDebug) \
    qDebug() << "***[" << Q_FUNC_INFO << "]***"
#define DbgPrintCB()                            \
    if (info->logLevel() >= QUsbInfo::logDebug) \
    qDebug() << "***[" << Q_FUNC_INFO << "]***"

static libusb_hotplug_callback_handle callback_handle;

static int LIBUSB_CALL hotplugCallback(libusb_context *ctx,
                                       libusb_device *device,
                                       libusb_hotplug_event event,
                                       void *user_data)
{
    struct libusb_device_descriptor desc;
    (void)ctx;
    QUsbInfo::IdList device_list;
    QUsbInfo::Id id;
    QUsbInfo *info = reinterpret_cast<QUsbInfo *>(user_data);
    DbgPrintCB();

    uint8_t bus = libusb_get_bus_number(device);
    uint8_t port = libusb_get_port_number(device);

    if (info->logLevel() >= QUsbInfo::logDebug)
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
        if (info->logLevel() >= QUsbInfo::logWarning)
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

/*!
    \enum QUsbInfo::LogLevel

\value logNone      No debug output
    \value logError     Errors only
    \value logWarning   Warning and abose
    \value logInfo      Info and above
    \value logDebug     Everything
    \value logDebugAll  Everything + libusb debug output
                                             */

/*!
    \property QUsbInfo::config
    \property QUsbInfo::id
    \property QUsbInfo::logLevel
    \property QUsbDevice::pid
    \property QUsbDevice::vid
    \property QUsbDevice::speed
    \property QUsbDevice::timeout

    \brief Various properties.
                                                     */

/*!
    \typedef QUsbDevice::Endpoint
    \brief An endpoint ID.
 */

/*!
    \typedef QUsbInfo::ConfigList
    \brief List of Config structs.
 */

/*!
    \typedef QUsbInfo::IdList
    \brief List of Id structs.
 */

/*!
    \class QUsbInfo::Config
    \brief Device configuration structure.
    \ingroup usb-main
    \inmodule QtUsb
 */

/*!
    \variable QUsbInfo::Config::config
    \brief The configuration ID.
 */

/*!
    \variable QUsbInfo::Config::interface
    \brief The interface ID.
 */

/*!
    \variable QUsbInfo::Config::alternate
    \brief The alternate ID.
 */

/*!
    \class QUsbInfo::Id
    \brief Device Ids structure.

                If some properties are equal to 0, they won't be taken into account for filtering.
        You only need PID and VID, or class and subclass to identify a device, but can be more specific if multiple devices using the same IDs are connected.
    \ingroup usb-main
    \inmodule QtUsb
                                                            */

/*!
    \variable QUsbInfo::Id::vid
    \brief The vendor ID.
*/

/*!
    \variable QUsbInfo::Id::pid
    \brief The product ID.
*/

/*!
    \variable QUsbInfo::Id::bus
    \brief The USB bus number.

                Default is \a QUsbDevice::busAny, which matches all buses.
                */

/*!
    \variable QUsbInfo::Id::port
    \brief The USB port number.

        Default is \a QUsbDevice::portAny, which matches all ports.
                */

/*!
    \variable QUsbInfo::Id::dClass
    \brief The USB class.
*/

/*!
    \variable QUsbInfo::Id::dSubClass
    \brief The USB Sub-class.
*/

QUsbInfo::QUsbInfo(QObject *parent)
    : QObject(*(new QUsbInfoPrivate), parent), d_dummy(Q_NULLPTR)
{
    Q_D(QUsbInfo);

    m_log_level = QUsbInfo::logInfo;
    DbgPrintFuncName();
    int rc;

    qRegisterMetaType<QUsbInfo::Id>("QUsbInfo::Id");
    qRegisterMetaType<QUsbInfo::Config>("QUsbInfo::Config");

    qRegisterMetaType<QUsbInfo::IdList>("QUsbInfo::IdList");
    qRegisterMetaType<QUsbInfo::ConfigList>("QUsbInfo::ConfigList");

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
    QUsbInfo::IdList list;

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
QUsbInfo::IdList QUsbInfo::devices()
{
    QUsbInfo::IdList list;
    ssize_t cnt; // holding number of devices in list
    libusb_device **devs;
    libusb_context *ctx;
    struct hid_device_info *hid_devs, *cur_hid_dev;

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
            QUsbInfo::Id id;
            id.pid = desc.idProduct;
            id.vid = desc.idVendor;
            id.bus = libusb_get_bus_number(dev);
            id.port = libusb_get_port_number(dev);

            list.append(id);
        }
    }

    libusb_free_device_list(devs, 1);
    libusb_exit(ctx);

    hid_devs = hid_enumerate(0x0, 0x0);
    cur_hid_dev = hid_devs;
    while (cur_hid_dev) {

        QUsbInfo::Id id;
        id.pid = cur_hid_dev->product_id;
        id.vid = cur_hid_dev->vendor_id;
        id.bus = 0;
        id.port = 0;

        list.append(id);

        cur_hid_dev = hid_devs->next;
    }
    hid_free_enumeration(hid_devs);

    return list;
}

/*!
      Check if \a id  device is present.

      Return bool true if present.
 */
bool QUsbInfo::isPresent(const QUsbInfo::Id &id) const
{
    DbgPrintFuncName();
    return this->findDevice(id, m_system_list) >= 0;
}

/*!
      Add an \a id device to the list.

      Returns false if device was already in the list, else true.
 */
bool QUsbInfo::addDevice(const QUsbInfo::Id &id)
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
bool QUsbInfo::removeDevice(const QUsbInfo::Id &id)
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
int QUsbInfo::findDevice(const QUsbInfo::Id &id,
                         const QUsbInfo::IdList &list) const
{
    DbgPrintFuncName();
    for (int i = 0; i < list.length(); i++) {
        const QUsbInfo::Id *d = &list.at(i);

        if (d->pid == id.pid && d->vid == id.vid) {
            if (id.bus == QUsbInfo::busAny && id.port == QUsbInfo::portAny) // Ignore bus/port if both == any
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
void QUsbInfo::setLogLevel(QUsbInfo::LogLevel level)
{
    DbgPrintFuncName();
    Q_D(QUsbInfo);
    m_log_level = level;
    if (m_log_level >= QUsbInfo::logDebug)
        libusb_set_debug(d->m_ctx, LIBUSB_LOG_LEVEL_DEBUG);
    else if (m_log_level >= QUsbInfo::logWarning)
        libusb_set_debug(d->m_ctx, LIBUSB_LOG_LEVEL_WARNING);
    else
        libusb_set_debug(d->m_ctx, LIBUSB_LOG_LEVEL_ERROR);
}

/*!
    Get current log level.
 */
QUsbInfo::LogLevel QUsbInfo::logLevel() const
{
    return m_log_level;
}

/*!
    Add a \a list to monitor.
 */
void QUsbInfo::monitorDevices(const QUsbInfo::IdList &list)
{

    DbgPrintFuncName();
    QUsbInfo::IdList inserted, removed;
    QUsbInfo::Id filter;

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

/*!
    \brief Comparision operator.

    Returns \c true if all \a other attributes match.
*/
bool QUsbInfo::Config::operator==(const QUsbInfo::Config &other) const
{
    return other.config == config && other.interface == interface && other.alternate == alternate;
}

QUsbInfo::Config::operator QString() const
{
    return QString::fromUtf8("Config(Config: %1, Interface: %2, Alternate: %3)").arg(config).arg(interface).arg(alternate);
}

/*!
    \brief Default constructor.
*/
QUsbInfo::Config::Config(quint8 _config, quint8 _interface, quint8 _alternate)
    : config(_config), interface(_interface), alternate(_alternate)
{
}

/*!
    \brief Copy constructor.
*/
QUsbInfo::Config::Config(const QUsbInfo::Config &other)
    : config(other.config), interface(other.interface), alternate(other.alternate)
{
}

/*!
    \brief Copy operator.
*/
QUsbInfo::Config &QUsbInfo::Config::operator=(QUsbInfo::Config other)
{
    config = other.config;
    alternate = other.alternate;
    interface = other.interface;
    return *this;
}

/*!
    \brief Default constructor.
*/
QUsbInfo::Id::Id(quint16 _pid, quint16 _vid, quint8 _bus, quint8 _port, quint8 _class, quint8 _subclass)
    : pid(_pid), vid(_vid), bus(_bus), port(_port), dClass(_class), dSubClass(_subclass)
{
}

/*!
    \brief Copy constructor.
*/
QUsbInfo::Id::Id(const QUsbInfo::Id &other)
    : pid(other.pid), vid(other.vid), bus(other.bus), port(other.port), dClass(other.dClass), dSubClass(other.dSubClass)
{
}

/*!
    \brief Comparision operator.

    Returns \c true if all \a other attributes match.
*/
bool QUsbInfo::Id::operator==(const QUsbInfo::Id &other) const
{
    return other.pid == pid
            && other.vid == vid
            && other.bus == bus
            && other.port == port
            && other.dClass == dClass
            && other.dSubClass == dSubClass;
}

/*!
    \brief Copy operator.
 */
QUsbInfo::Id &QUsbInfo::Id::operator=(QUsbInfo::Id other)
{
    pid = other.pid;
    vid = other.vid;
    bus = other.bus;
    port = other.port;
    dClass = other.dClass;
    dSubClass = other.dSubClass;
    return *this;
}

QUsbInfo::Id::operator QString() const
{
    return QString::fromUtf8("Id(Vid: %1, Pid: %2, Bus: %3, Port: %4, Class: %5, Subclass: %6)")
            .arg(vid, 4, 16, QChar::fromLatin1('0'))
            .arg(pid, 4, 16, QChar::fromLatin1('0'))
            .arg(bus)
            .arg(port)
            .arg(dClass)
            .arg(dSubClass);
}
