#include "qusbdevice.h"
#include "qusbdevice_p.h"
#include <QElapsedTimer>

#define DbgPrintError() qWarning("In %s, at %s:%d", Q_FUNC_INFO, __FILE__, __LINE__)
#define DbgPrintFuncName()       \
    if (m_log_level >= QUsb::logDebug) \
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

    if (dev->pub->logLevel() >= QUsb::logDebug)
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

QUsbDevice::QUsbDevice(QObject *parent)
    : QObject(*(new QUsbDevicePrivate), parent), d_dummy(Q_NULLPTR)
{
    m_spd = QUsb::unknownSpeed;
    m_connected = false;
    m_log_level = QUsb::logInfo;
    m_timeout = DefaultTimeout;
    m_config.config = 0x01;
    m_config.interface = 0x00;
    m_config.alternate = 0x00;
    m_status = QUsb::statusOK;
    this->setLogLevel(m_log_level); // Apply log level to libusb

    qRegisterMetaType<QUsb::DeviceStatus>("QUsbDevice::DeviceStatus");

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
    case QUsb::unknownSpeed:
        return "Unknown speed";
    case QUsb::lowSpeed:
        return "Low speed";
    case QUsb::fullSpeed:
        return "Full speed";
    case QUsb::highSpeed:
        return "High speed";
    case QUsb::superSpeed:
        return "Super speed";
    case QUsb::superSpeedPlus:
        return "Super speed plus";
    }

    return "Error";
}

QUsb::DeviceStatus QUsbDevice::status() const
{
    return m_status;
}

QByteArray QUsbDevice::statusString() const
{
    switch (m_status) {
    case QUsb::statusOK:
        return "Success (no error)";
    case QUsb::statusIoError:
        return "Input/output error";
    case QUsb::statusInvalidParam:
        return "Invalid parameter";
    case QUsb::statusAccessDenied:
        return "Access denied (insufficient permissions)";
    case QUsb::statusNoSuchDevice:
        return "No such device (it may have been disconnected)";
    case QUsb::statusNotFound:
        return "Entity not found";
    case QUsb::statusBusy:
        return "Resource busy";
    case QUsb::statusTimeout:
        return "Operation timed out";
    case QUsb::statusOverflow:
        return "Overflow";
    case QUsb::statusPipeError:
        return "Pipe error";
    case QUsb::statusInterrupted:
        return "System call interrupted (perhaps due to signal)";
    case QUsb::statusNoMemory:
        return "Insufficient memory";
    case QUsb::statusNotSupported:
        return "Operation not supported or unimplemented on this platform";
    case QUsb::statusUnknownError:
        break;
    }

    return "Other error";
}

void QUsbDevice::handleUsbError(int error_code)
{
    DbgPrintFuncName();
    QUsb::DeviceStatus status = static_cast<QUsb::DeviceStatus>(error_code);
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

    if ((m_id.pid == 0 || m_id.vid == 0) && (m_id.dClass == 0 || m_id.dSubClass == 0) && (m_id.bus == QUsb::busAny || m_id.port == QUsb::portAny)) {
        qWarning("No device IDs or classes are defined. Aborting.");
        return -1;
    }

    cnt = libusb_get_device_list(d->m_ctx, &d->m_devs); // get the list of devices
    if (cnt < 0) {
        qCritical("libusb_get_device_list error");
        libusb_free_device_list(d->m_devs, 1);
        return -1;
    }
    QList<int> interfaces;
    for (int i = 0; i < cnt; i++) {
        dev = d->m_devs[i];
        quint8 bus = libusb_get_bus_number(dev);
        quint8 port = libusb_get_port_number(dev);
        libusb_device_descriptor desc;

        if (libusb_get_device_descriptor(dev, &desc) == 0) {
            QUsb::Id tmp_id(m_id);
            // Assign default properties in order to match
            if (tmp_id.pid == 0)
                tmp_id.pid = desc.idProduct;
            if (tmp_id.vid == 0)
                tmp_id.vid = desc.idVendor;
            if (tmp_id.bus == QUsb::busAny)
                tmp_id.bus = bus;
            if (tmp_id.port == QUsb::portAny)
                tmp_id.port = port;
            if (tmp_id.dClass == 0)
                tmp_id.dClass = desc.bDeviceClass;
            if (tmp_id.dSubClass == 0)
                tmp_id.dSubClass = desc.bDeviceSubClass;
            // Check all properties match. Defaults have been assigned above.
            if (desc.idProduct == tmp_id.pid && desc.idVendor == tmp_id.vid
                && bus == tmp_id.bus && port == tmp_id.port
                && desc.bDeviceClass == tmp_id.dClass && desc.bDeviceSubClass == tmp_id.dSubClass) {
                if (m_log_level >= QUsb::logInfo)
                    qInfo() << "Found device:" << desc.iProduct << " @ " << tmp_id.vid << ":" << tmp_id.pid;
                libusb_config_descriptor* deviceConfiguration;
                QUsb::Config cg(m_config.config);
                if(libusb_get_config_descriptor(dev, m_config.config-1, &deviceConfiguration) == 0)
                {
                    if(deviceConfiguration)
                    {
                        auto interfaceCount = deviceConfiguration->bNumInterfaces;
                        auto interfaces = deviceConfiguration->interface;
                        for(quint8 i(0); i < interfaceCount; ++i )
                        {
                            auto interface = interfaces[i];
                            for(quint8 j(0); j < interface.num_altsetting; ++j){
                                auto interface_descriptor = interface.altsetting[j];
                                auto endpointCount = interface_descriptor.bNumEndpoints;
                                for(quint8 m(0); m < endpointCount; ++m)
                                {
                                    cg.interface = i;
                                    cg.alternate = j;
                                    auto endpoint_descriptor = interface_descriptor.endpoint[m];
                                    QUsb::EndpointDescription desc(endpoint_descriptor.bEndpointAddress, endpoint_descriptor.bmAttributes, cg.config, cg.interface, cg.alternate);
                                    m_endpoint_descriptions.append(desc);
                                }
                                    qDebug() << "Interface Details-> ConfigValue:" << deviceConfiguration->bConfigurationValue
                                             << " interface:" << i << " alternate:" << j;
                            }
                        }
                    }
                }
                rc = libusb_open(dev, &d->m_devHandle);
                if (rc == 0) {
                    m_id = tmp_id;
                    auto buffer = new unsigned char[512];
                    auto size = libusb_get_string_descriptor(d->m_devHandle, desc.iProduct, 0, buffer, 512);
                    if(size > 0) {
                        m_id.description = QString::fromUtf8(reinterpret_cast<char*>(buffer),size);
                        m_id.description.resize(size);
                    }
                    delete []buffer;
                    break;
                }
                else if (m_log_level >= QUsb::logWarning) {
                    qWarning("Failed to open device: %s", libusb_strerror(static_cast<enum libusb_error>(rc)));

                }
            }
        }
    }
    libusb_free_device_list(d->m_devs, 1); // free the list, unref the devices in it

    if (rc != 0 || d->m_devHandle == Q_NULLPTR) {
        return rc;
    }

    if (m_log_level >= QUsb::logInfo)
        qInfo("Device Open");
    for( const auto &endpoint : qAsConst(m_endpoint_descriptions)){
        if (libusb_kernel_driver_active(d->m_devHandle, endpoint.interface) == 1) { // find out if kernel driver is attached
            if (m_log_level >= QUsb::logDebug)
                qDebug("Kernel Driver Active");
            if (libusb_detach_kernel_driver(d->m_devHandle, endpoint.interface) == 0) // detach it
                if (m_log_level >= QUsb::logDebug)
                    qDebug("Kernel Driver Detached!");
        }
    }

    int conf;
    libusb_get_configuration(d->m_devHandle, &conf);

    if (conf != m_config.config) {
        if (m_log_level >= QUsb::logInfo)
            qInfo("Configuration needs to be changed");
        rc = libusb_set_configuration(d->m_devHandle, m_config.config-1);
        if (rc != 0) {
            if (m_log_level >= QUsb::logWarning)
                qWarning("Cannot Set Configuration");
            handleUsbError(rc);
            return -3;
        }
    }
    if(m_config.interface < 0){// initialize entire composite device
        QList<int> interfacesClaimed;
        for(const auto& endpoint: qAsConst(m_endpoint_descriptions)){
            if(!interfacesClaimed.contains(endpoint.interface)){
                m_config.interface = endpoint.interface;
                rc = libusb_claim_interface(d->m_devHandle, endpoint.interface);
                if (rc != 0) {
                    if (m_log_level >= QUsb::logWarning) {
                        qWarning() << "Cannot Claim Interface: " << endpoint.interface;
                        qWarning() << "alternate: " << m_config.alternate;
                        qWarning() << "config: " << m_config.config;
                    }
                    handleUsbError(rc);
                    return -4;
                }
                interfacesClaimed.append(endpoint.interface);
            }
        }
        m_config.interface = interfacesClaimed.size();

    }else{
        rc = libusb_claim_interface(d->m_devHandle, m_config.interface);
        if (rc != 0) {
            if (m_log_level >= QUsb::logWarning) {
                qWarning() << "Cannot Claim Interface: " << m_config.interface;
                qWarning() << "alternate: " << m_config.alternate;
                qWarning() << "config: " << m_config.config;
            }
            handleUsbError(rc);
            return -4;
        }
    }

    switch (libusb_get_device_speed(dev)) {
    case LIBUSB_SPEED_LOW:
        this->m_spd = QUsb::lowSpeed;
        break;
    case LIBUSB_SPEED_FULL:
        this->m_spd = QUsb::fullSpeed;
        break;
    case LIBUSB_SPEED_HIGH:
        this->m_spd = QUsb::highSpeed;
        break;
    case LIBUSB_SPEED_SUPER:
        this->m_spd = QUsb::superSpeed;
        break;
    default:
        this->m_spd = QUsb::unknownSpeed;
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
        if (m_log_level >= QUsb::logInfo)
            qInfo("Closing USB connection");

        libusb_release_interface(d->m_devHandle, 0); // release the claimed interface
        libusb_close(d->m_devHandle); // close the device we opened
        d->m_events->exit(0); // stop event handling thread
        d->m_events->wait();
        d->m_devHandle = Q_NULLPTR;
        m_connected = false;
        emit connectionChanged(m_connected);
    } else { // do not emit signal if device is already closed.
        if (m_log_level >= QUsb::logInfo)
            qInfo("USB connection already closed");
    }
}

/*!
    \brief Set the log \a level.
 */
void QUsbDevice::setLogLevel(QUsb::LogLevel level)
{
    Q_D(QUsbDevice);
    m_log_level = level;
    if (level >= QUsb::logDebugAll)
        libusb_set_option(d->m_ctx, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_DEBUG);
    else
        libusb_set_option(d->m_ctx, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_NONE);
}

/*!
    \brief Set the device \a id.
 */
void QUsbDevice::setId(const QUsb::Id &id)
{
    if(!(m_id == id)){
        m_id = id;
        emit idChanged(m_id);
    }

}

/*!
    \brief Set the device \a config.
 */
void QUsbDevice::setConfig(const QUsb::Config &config)
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
QUsb::Id QUsbDevice::id() const
{
    return m_id;
}

/*!
    \brief Returns the current \c config.
 */
QUsb::Config QUsbDevice::config() const
{
    return m_config;
}


const QUsb::EndpointDescriptionList& QUsbDevice::endpointDescriptions() const
{
    return m_endpoint_descriptions;
}

quint16 QUsbDevice::endpointCount() const
{
    return m_endpoint_descriptions.size();
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
QUsb::LogLevel QUsbDevice::logLevel() const
{
    return m_log_level;
}

/*!
    \brief Returns the device \c speed.
 */
QUsb::DeviceSpeed QUsbDevice::speed() const
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
