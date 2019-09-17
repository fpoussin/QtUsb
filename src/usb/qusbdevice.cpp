#include "qusbdevice.h"
#include "qusbdevice_p.h"
#include <QElapsedTimer>

#define DbgPrintError() qWarning("In %s, at %s:%d", Q_FUNC_INFO, __FILE__, __LINE__)
#define DbgPrintFuncName()       \
    if (m_log_level >= logDebug) \
    qDebug() << "***[" << Q_FUNC_INFO << "]***"

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
}

/*!
    \brief Destructor.
 */
QUsbDevice::~QUsbDevice()
{
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

/*!
    \brief Returns all present \c devices.
 */
QUsbDevice::IdList QUsbDevice::devices()
{
    IdList list;
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
            Id filter;
            filter.pid = desc.idProduct;
            filter.vid = desc.idVendor;

            list.append(filter);
        }
    }

    libusb_free_device_list(devs, 1);
    libusb_exit(ctx);
    return list;
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

    cnt = libusb_get_device_list(d->m_ctx, &d->m_devs); // get the list of devices
    if (cnt < 0) {
        qCritical("libusb_get_device_list error");
        libusb_free_device_list(d->m_devs, 1);
        return -1;
    }

    for (int i = 0; i < cnt; i++) {
        dev = d->m_devs[i];
        libusb_device_descriptor desc;

        if (libusb_get_device_descriptor(dev, &desc) == 0) {
            if (desc.idProduct == m_id.pid && desc.idVendor == m_id.vid) {
                if (m_log_level >= logInfo)
                    qInfo("Found device");

                rc = libusb_open(dev, &d->m_devHandle);
                if (rc == 0)
                    break;
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
            d->printUsbError(rc);
            return -3;
        }
    }
    rc = libusb_claim_interface(d->m_devHandle, m_config.interface);
    if (rc != 0) {
        if (m_log_level >= logWarning)
            qWarning("Cannot Claim Interface");
        d->printUsbError(rc);
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
        d->m_devHandle = Q_NULLPTR;
    }

    m_connected = false;
    emit connectionChanged(m_connected);
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
    return other.config == config &&
           other.interface == interface &&
           other.alternate == alternate;
}

QUsbDevice::Config &QUsbDevice::Config::operator=(const QUsbDevice::Config &other)
{
    config = other.config;
    alternate = other.alternate;
    interface = other.interface;
    return *this;
}

/*!
    \brief Comparision operator.

    Returns \c true if all \a other attributes match.
 */
bool QUsbDevice::Id::operator==(const QUsbDevice::Id &other) const
{
    return other.pid == pid &&
           other.vid == vid;
}

QUsbDevice::Id &QUsbDevice::Id::operator=(const QUsbDevice::Id &other)
{
    pid = other.pid;
    vid = other.vid;
    return *this;
}
