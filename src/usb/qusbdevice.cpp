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
    this->setLogLevel(m_log_level); // Apply log level to libusb
}

QUsbDevice::~QUsbDevice()
{
}

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

    return 0;
}

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
}

void QUsbDevice::setLogLevel(LogLevel level)
{
    Q_D(QUsbDevice);
    m_log_level = level;
    if (level >= logDebugAll)
        libusb_set_debug(d->m_ctx, LIBUSB_LOG_LEVEL_DEBUG);
    else
        libusb_set_debug(d->m_ctx, LIBUSB_LOG_LEVEL_NONE);
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
