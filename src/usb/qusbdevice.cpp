#include "qusbdevice.h"
#include "qusbdevice_p.h"
#include <QElapsedTimer>

#define DbgPrintError() qWarning("In %s, at %s:%d", Q_FUNC_INFO, __FILE__, __LINE__)
#define DbgPrintFuncName()       \
    if (m_log_level >= logDebug) \
    qDebug() << "***[" << Q_FUNC_INFO << "]***"

static libusb_hotplug_callback_handle callback_handle;

static int LIBUSB_CALL DeviceLeftCallback(libusb_context *ctx,
                                          libusb_device *device,
                                          libusb_hotplug_event event,
                                          void *user_data)
{
    (void)ctx;
    struct libusb_device_descriptor desc;
    (void)libusb_get_device_descriptor(device, &desc);
    qusbdevice_classes_t *dev = reinterpret_cast<qusbdevice_classes_t *>(user_data);

    if (dev->pub->logLevel() >= QUsbDevice::logDebug)
        qDebug("DeviceLeftCallback");

    if (event == LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT) {
        if (dev->priv->m_ctx == ctx && *dev->priv->m_devs == device)
            dev->pub->close();
    }
    return 0;
}

QUsbDevicePrivate::QUsbDevicePrivate()
{
    int rc = libusb_init(&m_ctx);
    if (rc < 0) {
        qCritical("LibUsb Init Error %d", rc);
    }
    Q_Q(QUsbDevice);
    m_devHandle = Q_NULLPTR;
    m_classes = { this, q };

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

/*!
    \class QUsbDevice

    \brief This class handles opening and configuring the device.

    \reentrant
    \ingroup usb-main
    \inmodule QtUsb
 */

/*!
    \enum QUsbDevice::DeviceSpeed

    \value unknownSpeed     Speed is unkown
    \value lowSpeed         USB 1.0
    \value fullSpeed        USB 1.1/2.0
    \value highSpeed        USB 2.0
    \value superSpeed       USB 3.0/3.1 G1
    \value superSpeedPlus   USB 3.1 G2
 */

/*!
    \enum QUsbDevice::DeviceStatus

    \value statusOK             Success (no error)
    \value statusIoError        Input/output error
    \value statusInvalidParam   Invalid parameter
    \value statusAccessDenied   Access denied (insufficient permissions)
    \value statusNoSuchDevice   No such device (it may have been disconnected)
    \value statusNotFound       Entity not found
    \value statusBusy           Resource busy
    \value statusTimeout        Operation timed out
    \value statusOverflow       Overflow
    \value statusPipeError      Pipe error
    \value statusInterrupted    System call interrupted (perhaps due to signal)
    \value statusNoMemory       Insufficient memory
    \value statusNotSupported   Operation not supported or unimplemented on this platform
    \value StatusUnknownError   Other error
 */

/*!
    \enum QUsbDevice::LogLevel

    \value logNone      No debug output
    \value logError     Errors only
    \value logWarning   Warning and abose
    \value logInfo      Info and above
    \value logDebug     Everything
    \value logDebugAll  Everything + libusb debug output
 */

/*!
    \property QUsbDevice::config
    \property QUsbDevice::id
    \property QUsbDevice::logLevel
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
    \typedef QUsbDevice::ConfigList
    \brief List of Config structs.
 */

/*!
    \typedef QUsbDevice::IdList
    \brief List of Id structs.
 */

/*!
    \class QUsbDevice::Config
    \brief Device configuration structure.
    \ingroup usb-main
    \inmodule QtUsb
 */

/*!
    \variable QUsbDevice::Config::config
    \brief The configuration ID.
 */

/*!
    \variable QUsbDevice::Config::interface
    \brief The interface ID.
 */

/*!
    \variable QUsbDevice::Config::alternate
    \brief The alternate ID.
 */

/*!
    \class QUsbDevice::Id
    \brief Device Ids structure.

    If some properties are equal to 0, they won't be taken into account for filtering.
    You only need PID and VID, or class and subclass to identify a device, but can be more specific if multiple devices using the same IDs are connected.
    \ingroup usb-main
    \inmodule QtUsb
 */

/*!
    \variable QUsbDevice::Id::vid
    \brief The vendor ID.
 */

/*!
    \variable QUsbDevice::Id::pid
    \brief The product ID.
 */

/*!
    \variable QUsbDevice::Id::bus
    \brief The USB bus number.

    Default is \a QUsbDevice::busAny, which matches all buses.
 */

/*!
    \variable QUsbDevice::Id::port
    \brief The USB port number.

    Default is \a QUsbDevice::portAny, which matches all ports.
 */

/*!
    \variable QUsbDevice::Id::dClass
    \brief The USB class.
 */

/*!
    \variable QUsbDevice::Id::dSubClass
    \brief The USB Sub-class.
 */

QUsbDevice::QUsbDevice(QObject *parent)
    : QObject(*(new QUsbDevicePrivate), parent), d_dummy(Q_NULLPTR)
{
    m_spd = unknownSpeed;
    m_connected = false;
    m_log_level = logInfo;
    m_timeout = DefaultTimeout;
    m_config.config = 0x01;
    m_config.interface = 0x00;
    m_config.alternate = 0x00;
    m_status = statusOK;
    this->setLogLevel(m_log_level); // Apply log level to libusb

    Q_D(QUsbDevice);
    if (libusb_has_capability(LIBUSB_CAP_HAS_HOTPLUG) != 0) {

        int rc;
        rc = libusb_hotplug_register_callback(d->m_ctx,
                                              static_cast<libusb_hotplug_event>(LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT),
                                              LIBUSB_HOTPLUG_ENUMERATE,
                                              m_id.vid,
                                              m_id.pid,
                                              LIBUSB_HOTPLUG_MATCH_ANY,
                                              reinterpret_cast<libusb_hotplug_callback_fn>(DeviceLeftCallback),
                                              reinterpret_cast<void *>(&d->m_classes),
                                              &callback_handle);
        if (LIBUSB_SUCCESS != rc) {
            libusb_exit(d->m_ctx);
            qWarning("Error creating hotplug callback");
            return;
        }
    }
}

/*!
    \brief Destructor.
 */
QUsbDevice::~QUsbDevice()
{
    this->close();
}

/*!
    \brief Returns the current speed as a human readable \c string.
 */
QByteArray QUsbDevice::speedString() const
{
    switch (m_spd) {
    case unknownSpeed:
        return "Unknown speed";
    case lowSpeed:
        return "Low speed";
    case fullSpeed:
        return "Full speed";
    case highSpeed:
        return "High speed";
    case superSpeed:
        return "Super speed";
    case superSpeedPlus:
        return "Super speed plus";
    }

    return "Error";
}

QUsbDevice::DeviceStatus QUsbDevice::status() const
{
    return m_status;
}

QByteArray QUsbDevice::statusString() const
{
    switch (m_status) {
    case statusOK:
        return "Success (no error)";
    case statusIoError:
        return "Input/output error";
    case statusInvalidParam:
        return "Invalid parameter";
    case statusAccessDenied:
        return "Access denied (insufficient permissions)";
    case statusNoSuchDevice:
        return "No such device (it may have been disconnected)";
    case statusNotFound:
        return "Entity not found";
    case statusBusy:
        return "Resource busy";
    case statusTimeout:
        return "Operation timed out";
    case statusOverflow:
        return "Overflow";
    case statusPipeError:
        return "Pipe error";
    case statusInterrupted:
        return "System call interrupted (perhaps due to signal)";
    case statusNoMemory:
        return "Insufficient memory";
    case statusNotSupported:
        return "Operation not supported or unimplemented on this platform";
    case statusUnknownError:
        break;
    }

    return "Other error";
}

void QUsbDevice::handleUsbError(int error_code)
{
    DbgPrintFuncName();
    DeviceStatus status = static_cast<QUsbDevice::DeviceStatus>(error_code);
    if (status != m_status) {
        m_status = status;
        qWarning("Usb device status changed: %s", statusString().constData());
        emit statusChanged(m_status);
    }
}

/*!
    \brief Open the device. Returns \c 0 on success
 */
qint32 QUsbDevice::open()
{
    DbgPrintFuncName();
    Q_D(QUsbDevice);

    int rc = -5; // Not found by default
    ssize_t cnt; // holding number of devices in list
    libusb_device *dev = Q_NULLPTR;

    if (m_connected)
        return -1;

    if ((m_id.pid == 0 || m_id.vid == 0) && (m_id.dClass == 0 || m_id.dSubClass == 0) && (m_id.bus == busAny || m_id.port == portAny)) {
        qWarning("No device IDs or classes are defined. Aborting.");
        return -1;
    }

    cnt = libusb_get_device_list(d->m_ctx, &d->m_devs); // get the list of devices
    if (cnt < 0) {
        qCritical("libusb_get_device_list error");
        libusb_free_device_list(d->m_devs, 1);
        return -1;
    }

    for (int i = 0; i < cnt; i++) {
        dev = d->m_devs[i];
        quint8 bus = libusb_get_bus_number(dev);
        quint8 port = libusb_get_port_number(dev);
        libusb_device_descriptor desc;

        if (libusb_get_device_descriptor(dev, &desc) == 0) {
            Id tmp_id(m_id);
            // Assign default properties in order to match
            if (tmp_id.pid == 0)
                tmp_id.pid = desc.idProduct;
            if (tmp_id.vid == 0)
                tmp_id.vid = desc.idVendor;
            if (tmp_id.bus == busAny)
                tmp_id.bus = bus;
            if (tmp_id.port == portAny)
                tmp_id.port = port;
            if (tmp_id.dClass == 0)
                tmp_id.dClass = desc.bDeviceClass;
            if (tmp_id.dSubClass == 0)
                tmp_id.dSubClass = desc.bDeviceSubClass;

            // Check all properties match. Defaults have been assigned above.
            if (desc.idProduct == tmp_id.pid && desc.idVendor == tmp_id.vid
                && bus == tmp_id.bus && port == tmp_id.port
                && desc.bDeviceClass == tmp_id.dClass && desc.bDeviceSubClass == tmp_id.dSubClass) {
                if (m_log_level >= logInfo)
                    qInfo("Found device");

                rc = libusb_open(dev, &d->m_devHandle);
                if (rc == 0) {
                    m_id = tmp_id;
                    break;
                }
                else if (m_log_level >= logWarning) {
                    qWarning("Failed to open device: %s", libusb_strerror(static_cast<enum libusb_error>(rc)));
                }
            }
        }
    }
    libusb_free_device_list(d->m_devs, 1); // free the list, unref the devices in it

    if (rc != 0 || d->m_devHandle == Q_NULLPTR) {
        return rc;
    }

    if (m_log_level >= logInfo)
        qInfo("Device Open");

    if (libusb_kernel_driver_active(d->m_devHandle, m_config.interface) == 1) { // find out if kernel driver is attached
        if (m_log_level >= logDebug)
            qDebug("Kernel Driver Active");
        if (libusb_detach_kernel_driver(d->m_devHandle, m_config.interface) == 0) // detach it
            if (m_log_level >= logDebug)
                qDebug("Kernel Driver Detached!");
    }

    int conf;
    libusb_get_configuration(d->m_devHandle, &conf);

    if (conf != m_config.config) {
        if (m_log_level >= logInfo)
            qInfo("Configuration needs to be changed");
        rc = libusb_set_configuration(d->m_devHandle, m_config.config);
        if (rc != 0) {
            if (m_log_level >= logWarning)
                qWarning("Cannot Set Configuration");
            handleUsbError(rc);
            return -3;
        }
    }
    rc = libusb_claim_interface(d->m_devHandle, m_config.interface);
    if (rc != 0) {
        if (m_log_level >= logWarning)
            qWarning("Cannot Claim Interface");
        handleUsbError(rc);
        return -4;
    }

    switch (libusb_get_device_speed(dev)) {
    case LIBUSB_SPEED_LOW:
        this->m_spd = lowSpeed;
        break;
    case LIBUSB_SPEED_FULL:
        this->m_spd = fullSpeed;
        break;
    case LIBUSB_SPEED_HIGH:
        this->m_spd = highSpeed;
        break;
    case LIBUSB_SPEED_SUPER:
        this->m_spd = superSpeed;
        break;
    default:
        this->m_spd = unknownSpeed;
        break;
    }

    if (!d->m_events->isRunning()) // if event handling thread is not running start it. The thread was stopped upon closing the device.
        d->m_events->start();

    m_connected = true;
    emit connectionChanged(m_connected);

    return 0;
}

/*!
    \brief Close the device.
 */
void QUsbDevice::close()
{
    DbgPrintFuncName();
    Q_D(QUsbDevice);

    if (d->m_devHandle && m_connected) {
        // stop any further write attempts whilst we close down
        if (m_log_level >= logInfo)
            qInfo("Closing USB connection");

        libusb_release_interface(d->m_devHandle, 0); // release the claimed interface
        libusb_close(d->m_devHandle); // close the device we opened
        d->m_events->exit(0); // stop event handling thread
        d->m_events->wait();
        d->m_devHandle = Q_NULLPTR;
        m_connected = false;
        emit connectionChanged(m_connected);
    } else { // do not emit signal if device is already closed.
        if (m_log_level >= logInfo)
            qInfo("USB connection already closed");
    }
}

/*!
    \brief Set the log \a level.
 */
void QUsbDevice::setLogLevel(LogLevel level)
{
    Q_D(QUsbDevice);
    m_log_level = level;
    if (level >= logDebugAll)
        libusb_set_debug(d->m_ctx, LIBUSB_LOG_LEVEL_DEBUG);
    else
        libusb_set_debug(d->m_ctx, LIBUSB_LOG_LEVEL_NONE);
}

/*!
    \brief Set the device \a id.
 */
void QUsbDevice::setId(const QUsbDevice::Id &id)
{
    m_id = id;
}

/*!
    \brief Set the device \a config.
 */
void QUsbDevice::setConfig(const QUsbDevice::Config &config)
{
    m_config = config;
}

/*!
    \brief Set the device \a timeout.
 */
void QUsbDevice::setTimeout(quint16 timeout)
{
    m_timeout = timeout;
}

/*!
    \brief Returns the device \c id.
 */
QUsbDevice::Id QUsbDevice::id() const
{
    return m_id;
}

/*!
    \brief Returns the current \c config.
 */
QUsbDevice::Config QUsbDevice::config() const
{
    return m_config;
}

/*!
    \brief Returns \c true if connected.
 */
bool QUsbDevice::isConnected() const
{
    return m_connected;
}

/*!
    \brief Return the device \c pid. (Product id)
 */
quint16 QUsbDevice::pid() const
{
    return m_id.pid;
}

/*!
    \brief Return the device \c vid. (Vendor id)
 */
quint16 QUsbDevice::vid() const
{
    return m_id.vid;
}

/*!
    \brief Return the \c timeout.
 */
quint16 QUsbDevice::timeout() const
{
    return m_timeout;
}

/*!
    \brief Returns the log \c level.
 */
QUsbDevice::LogLevel QUsbDevice::logLevel() const
{
    return m_log_level;
}

/*!
    \brief Returns the device \c speed.
 */
QUsbDevice::DeviceSpeed QUsbDevice::speed() const
{
    return m_spd;
}

void QUsbEventsThread::run()
{

    timeval t = { 0, 100000 };
    while (!this->isInterruptionRequested()) {
        if (libusb_event_handling_ok(m_ctx) == 0) {
            libusb_unlock_events(m_ctx);
            break;
        }
        if (libusb_handle_events_timeout_completed(m_ctx, &t, Q_NULLPTR) != 0) {
            break;
        }
    }
}

/*!
    \brief Comparision operator.

    Returns \c true if all \a other attributes match.
 */
bool QUsbDevice::Config::operator==(const QUsbDevice::Config &other) const
{
    return other.config == config && other.interface == interface && other.alternate == alternate;
}

QUsbDevice::Config::operator QString() const
{
    return QString::fromUtf8("Config(Config: %1, Interface: %2, Alternate: %3)").arg(config).arg(interface).arg(alternate);
}

/*!
    \brief Default constructor.
*/
QUsbDevice::Config::Config(quint8 _config, quint8 _interface, quint8 _alternate)
{
    config = _config;
    interface = _interface;
    alternate = _alternate;
}

/*!
    \brief Copy constructor.
*/
QUsbDevice::Config::Config(const QUsbDevice::Config &other)
{
    config = other.config;
    alternate = other.alternate;
    interface = other.interface;
}

/*!
    \brief Copy operator.
*/
QUsbDevice::Config &QUsbDevice::Config::operator=(QUsbDevice::Config other)
{
    config = other.config;
    alternate = other.alternate;
    interface = other.interface;
    return *this;
}

/*!
    \brief Default constructor.
*/
QUsbDevice::Id::Id(quint16 _pid, quint16 _vid, quint8 _bus, quint8 _port, quint8 _class, quint8 _subclass)
{
    pid = _pid;
    vid = _vid;
    bus = _bus;
    port = _port;
    dClass = _class;
    dSubClass = _subclass;
}

/*!
    \brief Copy constructor.
*/
QUsbDevice::Id::Id(const QUsbDevice::Id &other)
{
    pid = other.pid;
    vid = other.vid;
    bus = other.bus;
    port = other.port;
    dClass = other.dClass;
    dSubClass = other.dSubClass;
}

/*!
    \brief Comparision operator.

    Returns \c true if all \a other attributes match.
 */
bool QUsbDevice::Id::operator==(const QUsbDevice::Id &other) const
{
    return other.pid == pid && other.vid == vid && other.bus == bus && other.port == port && other.dClass == dClass && other.dSubClass == dSubClass;
}

/*!
    \brief Copy operator.
 */
QUsbDevice::Id &QUsbDevice::Id::operator=(QUsbDevice::Id other)
{
    pid = other.pid;
    vid = other.vid;
    bus = other.bus;
    port = other.port;
    dClass = other.dClass;
    dSubClass = other.dSubClass;
    return *this;
}

QUsbDevice::Id::operator QString() const
{
    return QString::fromUtf8("Id(Vid: %1, Pid: %2, Bus: %3, Port: %4, Class: %5, Subclass: %6)")
            .arg(vid, 4, 16, QChar::fromLatin1('0'))
            .arg(pid, 4, 16, QChar::fromLatin1('0'))
            .arg(bus)
            .arg(port)
            .arg(dClass)
            .arg(dSubClass);
}
