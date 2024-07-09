#include "qusb.h"
#include "qusb_p.h"
#include <QDebug>
#include <QThread>
#include <QMutexLocker>
#include <QMutex>

Q_LOGGING_CATEGORY(qUsb, "qusb")

#define DbgPrintError() qCWarning(qUsb, "In %s, at %s:%d", Q_FUNC_INFO, __FILE__, __LINE__)
#define DbgPrintFuncName()             \
    if (m_log_level >= QUsb::logDebug) \
    qCDebug(qUsb) << "***[" << Q_FUNC_INFO << "]***"
#define DbgPrintCB()                        \
    if (info->logLevel() >= QUsb::logDebug) \
    qCDebug(qUsb) << "***[" << Q_FUNC_INFO << "]***"

static libusb_hotplug_callback_handle callback_handle;
static QMutex g_mtx_hid_enumerate; // protects calls to `hid_enumerate` and `hid_free_enumeration`

static int LIBUSB_CALL hotplugCallback(libusb_context *ctx,
                                       libusb_device *device,
                                       libusb_hotplug_event event,
                                       void *user_data)
{
    struct libusb_device_descriptor desc;
    (void)ctx;
    QUsb::IdList device_list;
    QUsb::Id id;
    QUsb *info = reinterpret_cast<QUsb *>(user_data);
    DbgPrintCB();

    uint8_t bus = libusb_get_bus_number(device);
    uint8_t port = libusb_get_port_number(device);

    qCDebug(qUsb) << "hotplugCallback";

    libusb_get_device_descriptor(device, &desc);
    id.vid = desc.idVendor;
    id.pid = desc.idProduct;
    id.bus = bus;
    id.port = port;
    id.dClass = desc.bDeviceClass;
    id.dSubClass = desc.bDeviceSubClass;

    if (event == LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED) {
        qCDebug(qUsb) << "Hotplug - device inserted: " << id;
        // Add to list
        emit info->deviceInserted(id);

    } else if (event == LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT) {
        qCDebug(qUsb) << "Hotplug - device removed: " << id;
        // Remove from list
        emit info->deviceRemoved(id);

    } else {
        if (info->logLevel() >= QUsb::logWarning)
            qCWarning(qUsb, "Unhandled hotplug event %d", event);
        return -1;
    }
    return 0;
}

QUsbPrivate::QUsbPrivate()
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

QUsbPrivate::~QUsbPrivate()
{
    m_refresh_timer->thread()->exit();
    m_refresh_timer->thread()->wait();
    m_refresh_timer->thread()->deleteLater();

    m_refresh_timer->disconnect();
    m_refresh_timer->deleteLater();
}

/*!
    \class QUsb

    \brief This class handles hotplug and device detection.

    \reentrant
    \ingroup usb-main
    \inmodule QtUsb

    Handles USB events and searching.
    Can be used to monitor events for a list of devices or all system devices.

    \sa QUsbDevice
*/

/*!
    \fn void QUsb::deviceInserted(Id id)

    This is signal is emited when one or more new devices are detected, providing a \a list.
*/

/*!
    \fn void QUsb::deviceRemoved(Id id)

    This is signal is emited when one or more new devices are removed, providing a \a list.
*/

/*!
    \property QUsb::logLevel
    \brief the log level for hotplug/detection.
*/

/*!
    \enum QUsb::LogLevel

\value logNone      No debug output
    \value logError     Errors only
    \value logWarning   Warning and abose
    \value logInfo      Info and above
    \value logDebug     Everything
    \value logDebugAll  Everything + libusb debug output
                                             */

/*!
    \property QUsb::logLevel
    \property QUsbDevice::pid
    \property QUsbDevice::vid
    \property QUsbDevice::speed
    \property QUsbDevice::timeout

    \brief Various properties.
                                                     */

/*!
    \typedef QUsb::ConfigList
    \brief List of Config structs.
 */

/*!
    \typedef QUsb::IdList
    \brief List of Id structs.
 */

/*!
    \class QUsb::Config
    \brief Device configuration structure.
    \ingroup usb-main
    \inmodule QtUsb
 */

/*!
    \variable QUsb::Config::config
    \brief The configuration ID.
 */

/*!
    \variable QUsb::Config::interface
    \brief The interface ID.
 */

/*!
    \variable QUsb::Config::alternate
    \brief The alternate ID.
 */

/*!
    \class QUsb::Id
    \brief Device Ids structure.

                If some properties are equal to 0, they won't be taken into account for filtering.
        You only need PID and VID, or class and subclass to identify a device, but can be more specific if multiple devices using the same IDs are connected.
    \ingroup usb-main
    \inmodule QtUsb
                                                            */

/*!
    \variable QUsb::Id::vid
    \brief The vendor ID.
*/

/*!
    \variable QUsb::Id::pid
    \brief The product ID.
*/

/*!
    \variable QUsb::Id::bus
    \brief The USB bus number.

                Default is \a QUsbDevice::busAny, which matches all buses.
                */

/*!
    \variable QUsb::Id::port
    \brief The USB port number.

        Default is \a QUsbDevice::portAny, which matches all ports.
                */

/*!
    \variable QUsb::Id::dClass
    \brief The USB class.
*/

/*!
    \variable QUsb::Id::dSubClass
    \brief The USB Sub-class.
*/

QUsb::QUsb(QObject *parent)
    : QObject(*(new QUsbPrivate), parent), d_dummy(Q_NULLPTR)
{
    Q_D(QUsb);

    m_log_level = QUsb::logInfo;
    DbgPrintFuncName();
    int rc;

    qRegisterMetaType<QUsb::Id>("QUsb::Id");
    qRegisterMetaType<QUsb::Config>("QUsb::Config");

    qRegisterMetaType<QUsb::IdList>("QUsb::IdList");
    qRegisterMetaType<QUsb::ConfigList>("QUsb::ConfigList");

    qCDebug(qUsb) << "libusb_init";
    rc = libusb_init(&d->m_ctx);
    if (rc < 0) {
        libusb_exit(d->m_ctx);
        qCCritical(qUsb, "LibUsb Init Error %d", rc);
        return;
    }

    libusb_set_option(d->m_ctx, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_WARNING);

    // Populate list once
    m_system_list = devices();

    // Try hotplug first
    qCDebug(qUsb) << "Try hotplug first";
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
            qCWarning(qUsb, "Error creating hotplug callback");
            return;
        }
    }

    connect(d->m_refresh_timer, SIGNAL(timeout()), this, SLOT(checkDevices()));
    qCDebug(qUsb) << "QUsb created";
}

/*!
    Unregister callbacks and close the usb context.
 */
QUsb::~QUsb()
{
    Q_D(QUsb);
    DbgPrintFuncName();
    // Process any remaining events, then deregister hotplug callback
    if (d->m_has_hotplug) {
        timeval t = { 0, 0 };
        libusb_handle_events_timeout_completed(d->m_ctx, &t, Q_NULLPTR);
        libusb_hotplug_deregister_callback(d->m_ctx, callback_handle);
    }
    qCDebug(qUsb) << "libusb exit";
    libusb_exit(d->m_ctx);

    qCDebug(qUsb) << "QUsb destroyed";
}

/*!
    Check devices present in system.

    This gets called by the internal timer.
 */
void QUsb::checkDevices()
{
    DbgPrintFuncName();
    Q_D(QUsb);
    QUsb::IdList list;

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
QUsb::IdList QUsb::devices()
{
    QUsb::IdList list;
    ssize_t cnt; // holding number of devices in list
    libusb_device **devs;
    libusb_context *ctx;
    struct hid_device_info *hid_devs, *cur_hid_dev;

    qCDebug(qUsb) << "QUsb devices";

    libusb_init(&ctx);
    libusb_set_option(ctx, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_NONE);
    cnt = libusb_get_device_list(ctx, &devs); // get the list of devices
    if (cnt < 0) {
        qCCritical(qUsb, "libusb_get_device_list Error");
        libusb_free_device_list(devs, 1);
        return list;
    }
    qCDebug(qUsb) << "QUsb devices cnt " << cnt;

    for (int i = 0; i < cnt; i++) {
        libusb_device *dev = devs[i];
        libusb_device_descriptor desc;

        if (libusb_get_device_descriptor(dev, &desc) == 0) {
            QUsb::Id id;
            id.pid = desc.idProduct;
            id.vid = desc.idVendor;
            id.bus = libusb_get_bus_number(dev);
            id.port = libusb_get_port_number(dev);

            list.append(id);
        }
    }

    libusb_free_device_list(devs, 1);
    libusb_exit(ctx);

    qCDebug(qUsb) << "QUsb devices libusb_exit";

    {
        // NOTE: on some platforms hid_enumerate is not thread-safe, so we need an application-wide mutex
        QMutexLocker lock(&g_mtx_hid_enumerate);
        qCDebug(qUsb) << "QUsb devices mutex lock";

        hid_devs = hid_enumerate(0x0, 0x0);
        cur_hid_dev = hid_devs;
        while (cur_hid_dev) {

            QUsb::Id id;
            id.pid = cur_hid_dev->product_id;
            id.vid = cur_hid_dev->vendor_id;
            id.bus = 0;
            id.port = 0;

            list.append(id);

            cur_hid_dev = cur_hid_dev->next;
        }
        hid_free_enumeration(hid_devs);

        qCDebug(qUsb) << "QUsb devices mutex unlock";
    }

    return list;
}

/*!
      Check if \a id  device is present.

      Return bool true if present.
 */
bool QUsb::isPresent(const QUsb::Id &id) const
{
    DbgPrintFuncName();
    return this->findDevice(id, m_system_list) >= 0;
}

/*!
      Add an \a id device to the list.

      Returns false if device was already in the list, else true.
 */
bool QUsb::addDevice(const QUsb::Id &id)
{

    DbgPrintFuncName();
    if (this->findDevice(id, m_list) == -1) {
        m_list.append(id);
        return true;
    }
    qCDebug(qUsb) << "Could not add device id=" << id;
    return false;
}

/*!
      \brief Remove \a id device from the list.

      Return bool false if device was not in the list, else true.
 */
bool QUsb::removeDevice(const QUsb::Id &id)
{

    DbgPrintFuncName();
    const int pos = this->findDevice(id, m_list);
    // TODO: Seems bug in condition and returned value: pos = -1 if not found, else pos = index
    if (pos > 0) {
        m_list.removeAt(pos);
        return true;
    }
    qCDebug(qUsb) << "Could not removeDevice id=" << id;
    return true;
}

/*!
      Search an \a id device in a device \a list.

      Return index of the filter, returns -1 if not found.
 */
int QUsb::findDevice(const QUsb::Id &id,
                     const QUsb::IdList &list) const
{
    DbgPrintFuncName();
    for (int i = 0; i < list.length(); i++) {
        const QUsb::Id *d = &list.at(i);

        if (d->pid == id.pid && d->vid == id.vid) {
            if (id.bus == QUsb::busAny && id.port == QUsb::portAny) // Ignore bus/port if both == any
                return i;
            if (d->bus == id.bus && d->port == id.port) // Take bus/port into account for filtering when set
                return i;
        }
    }
    qCWarning(qUsb) << "Device with id=" << id << "was not found";
    return -1;
}

/*!
    Set log \a level (only hotplug/detection).
 */
void QUsb::setLogLevel(QUsb::LogLevel level)
{
    DbgPrintFuncName();
    Q_D(QUsb);
    m_log_level = level;
    if (m_log_level >= QUsb::logDebug)
        libusb_set_option(d->m_ctx, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_DEBUG);
    else if (m_log_level >= QUsb::logWarning)
        libusb_set_option(d->m_ctx, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_WARNING);
    else
        libusb_set_option(d->m_ctx, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_ERROR);
}

/*!
    Get current log level.
 */
QUsb::LogLevel QUsb::logLevel() const
{
    return m_log_level;
}

/*!
    Add a \a list to monitor.
 */
void QUsb::monitorDevices(const QUsb::IdList &list)
{

    DbgPrintFuncName();
    QUsb::IdList inserted, removed;
    QUsb::Id filter;

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
        qCDebug(qUsb) << "device inserted: " << inserted.at(i);
        emit deviceInserted(inserted.at(i));
    }

    for (int i = 0; i < removed.length(); i++) {
        qCDebug(qUsb) << "device removed: " << removed.at(i);
        emit deviceRemoved(removed.at(i));
    }

    m_system_list = list;
}

/*!
    \brief Comparision operator.

    Returns \c true if all \a other attributes match.
*/
bool QUsb::Config::operator==(const QUsb::Config &other) const
{
    return other.config == config && other.interface == interface && other.alternate == alternate;
}

QUsb::Config::operator QString() const
{
    return QString::fromUtf8("Config(Config: %1, Interface: %2, Alternate: %3)").arg(config).arg(interface).arg(alternate);
}

/*!
    \brief Default constructor.
*/
QUsb::Config::Config(quint8 _config, quint8 _interface, quint8 _alternate)
    : config(_config), interface(_interface), alternate(_alternate)
{
}

/*!
    \brief Copy constructor.
*/
QUsb::Config::Config(const QUsb::Config &other)
    : config(other.config), interface(other.interface), alternate(other.alternate)
{
}

/*!
    \brief Copy operator.
*/
QUsb::Config &QUsb::Config::operator=(QUsb::Config other)
{
    config = other.config;
    alternate = other.alternate;
    interface = other.interface;
    return *this;
}

/*!
    \brief Default constructor.
*/
QUsb::Id::Id(quint16 _pid, quint16 _vid, quint8 _bus, quint8 _port, quint8 _class, quint8 _subclass)
    : pid(_pid), vid(_vid), bus(_bus), port(_port), dClass(_class), dSubClass(_subclass)
{
}

/*!
    \brief Copy constructor.
*/
QUsb::Id::Id(const QUsb::Id &other)
    : pid(other.pid), vid(other.vid), bus(other.bus), port(other.port), dClass(other.dClass), dSubClass(other.dSubClass)
{
}

/*!
    \brief Comparision operator.

    Returns \c true if all \a other attributes match.
*/
bool QUsb::Id::operator==(const QUsb::Id &other) const
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
QUsb::Id &QUsb::Id::operator=(QUsb::Id other)
{
    pid = other.pid;
    vid = other.vid;
    bus = other.bus;
    port = other.port;
    dClass = other.dClass;
    dSubClass = other.dSubClass;
    return *this;
}

QUsb::Id::operator QString() const
{
    return QString::fromUtf8("Id(Vid: %1, Pid: %2, Bus: %3, Port: %4, Class: %5, Subclass: %6)")
            .arg(vid, 4, 16, QChar::fromLatin1('0'))
            .arg(pid, 4, 16, QChar::fromLatin1('0'))
            .arg(bus)
            .arg(port)
            .arg(dClass)
            .arg(dSubClass);
}
