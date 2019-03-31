#ifndef QUSBDEVICE_H
#define QUSBDEVICE_H

#include "qusbglobal.h"
#include <QByteArray>
#include <QDebug>
#include <QString>

QT_BEGIN_NAMESPACE

class QUsbDevicePrivate;
class QUsbTransfer;
class QUsbTransferPrivate;

/**
 * @brief
 *
 */
class Q_USB_EXPORT QUsbDevice : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QUsbDevice)

    friend QUsbTransfer;
    friend QUsbTransferPrivate;

public:
    static const quint16 DefaultTimeout = 250; /**< Default timeout in milliseconds */

    /**
     * @brief Endpoint type
     */
    typedef quint8 Endpoint;

    /**
   * @brief The LogLevel enum
   */
    enum LogLevel {
        logNone = 0,
        logError = 1,
        logWarning = 2,
        logInfo = 3,
        logDebug = 4,
        logDebugAll = 5 // Includes libusb debug output
    };

    /**
   * @brief bmRequestType field
   *
   */
    enum bmRequestType {
        requestStandard = (0x00 < 5),
        requestClass = (0x01 < 5),
        requestVendor = (0x02 < 5),
        requestReserved = (0x03 < 5),
        recipientDevice = 0x00,
        recipientInterface = 0x01,
        recipientEndpoint = 0x02,
        recipientOther = 0x03
    };

    /**
   * @brief bRequest field
   *
   */
    enum bRequest {
        requestGetStatus = 0x00,
        requestClearFeature = 0x01,
        requestSetFeature = 0x03,
        requestSetAddress = 0x05,
        requestGetDescriptor = 0x06,
        requestSetDescriptor = 0x07,
        requestGetConfiguration = 0x08,
        requestSetConfiguration = 0x09,
        requestGetInterface = 0x0A,
        requestSetInterface = 0x0B,
        requestSynchFrame = 0x0C,
        requestSetSel = 0x30,
        requestIsochDelay = 0x31
    };

    /**
   * @brief Device configuration
   *
   */
    typedef struct Config {
        quint8 config; /**< Configuration index */
        quint8 interface; /**< Interface index */
        quint8 alternate; /**< Alternate configuration index */

        /**
         * @brief operator ==
         * @param o
         * @return
         */
        bool operator==(const QUsbDevice::Config &o) const
        {
            return o.config == this->config && o.interface == this->interface && o.alternate == this->alternate;
        }

    } Config;

    /**
   * @brief Device filter
   *
   */
    typedef struct Id {
        quint16 pid; /**< Product ID */
        quint16 vid; /**< Vendor ID */

        /**
         * @brief operator ==
         * @param o
         * @return
         */
        bool operator==(const QUsbDevice::Id &o) const
        {
            return o.pid == this->pid && o.vid == this->vid;
        }

    } Id;

    /**
   * @brief List of device filters
   *
   */
    typedef QList<Id> IdList;
    /**
   * @brief List of device configs
   *
   */
    typedef QList<Config> ConfigList;

    /**
   * @brief USB speeds
   *
   */
    enum DeviceSpeed {
        unknownSpeed = -1,
        lowSpeed = 0, /* USB 1.0 */
        fullSpeed, /* USB 1.1/2.0 */
        highSpeed, /* USB 2.0 */
        superSpeed, /* USB 3.0/3.1G1 */
        superSpeedPlus /* USB 3.1G2 */
    };
    Q_ENUM(DeviceSpeed)

    /**
   * @brief Device status
   *
   */
    /* Mapped to libusb_error */
    enum DeviceStatus {
        /** Success (no error) */
        statusOK = 0,
        /** Input/output error */
        statusIoError = -1,
        /** Invalid parameter */
        statusInvalidParam = -2,
        /** Access denied (insufficient permissions) */
        statusAccessDenied = -3,
        /** No such device (it may have been disconnected) */
        statusNoSuchDevice = -4,
        /** Entity not found */
        statusNotFound = -5,
        /** Resource busy */
        statusBusy = -6,
        /** Operation timed out */
        statusTimeout = -7,
        /** Overflow */
        statusOverflow = -8,
        /** Pipe error */
        statusPipeError = -9,
        /** System call interrupted (perhaps due to signal) */
        statusInterrupted = -10,
        /** Insufficient memory */
        statusNoMemory = -11,
        /** Operation not supported or unimplemented on this platform */
        statusNotSupported = -12,
        /** Other error */
        StatusUnknownError = -99,
    };
    Q_ENUM(DeviceStatus)

    Q_PROPERTY(LogLevel logLevel READ logLevel WRITE setLogLevel) /**< loglevel */
    Q_PROPERTY(Id id READ id WRITE setId) /**< IDs */
    Q_PROPERTY(Config config READ config WRITE setConfig) /**< Config */
    Q_PROPERTY(quint16 pid READ pid) /**< Product ID */
    Q_PROPERTY(quint16 vid READ vid) /**< Vendor ID */
    Q_PROPERTY(quint16 timeout READ timeout WRITE setTimeout) /**< Transfer timeout */
    Q_PROPERTY(DeviceSpeed speed READ speed) /**< Device speed */

    /**
   * @brief constructor
   *
   * @param parent
   */
    explicit QUsbDevice(QObject *parent = Q_NULLPTR);
    ~QUsbDevice();

    /**
   * @brief Enable/Disable debug
   *
   * @param level log level
   */
    void setLogLevel(LogLevel level);

    /**
   * @brief Set device Id
   *
   * @param id to apply
   */
    void setId(const Id &id) { m_id = id; }

    /**
   * @brief Set device config
   *
   * @param config to apply
   */
    void setConfig(const Config &config) { m_config = config; }

    /**
   * @brief Set device timeout
   *
   * @param timeout Timeout in milliseconds
   */
    void setTimeout(quint16 timeout) { m_timeout = timeout; }

    /**
   * @brief Get current device Id
   *
   * @return Id
   */
    Id id(void) const { return m_id; }

    /**
   * @brief Get current device config
   *
   * @return DeviceConfig
   */
    Config config(void) const { return m_config; }

    /**
   * @brief Get connection status
   *
   * @return bool
   */
    bool isConnected(void) const { return m_connected; }

    /**
   * @brief Get current device pid
   *
   * @return quint16
   */
    quint16 pid(void) const { return m_id.pid; }

    /**
   * @brief Get current device vid
   *
   * @return quint16
   */
    quint16 vid(void) const { return m_id.vid; }

    /**
   * @brief Get current timeout
   *
   * @return quint8
   */
    quint16 timeout(void) const { return m_timeout; }

    /**
   * @brief Get debug mode
   *
   * @return bool debug enabled
   */
    LogLevel logLevel(void) const { return m_log_level; }

    /**
   * @brief Get current device speed
   *
   * @return DeviceSpeed
   */
    DeviceSpeed speed(void) const { return m_spd; }

    /**
   * @brief Get current device speed string
   *
   * @return QByteArray
   */
    QByteArray speedString(void) const;

    /**
   * @brief Get a list of all USB devices available for use
   * Static function
   *
   * @return FilterList
   */
    static IdList devices(void);

public slots:
    /**
   * @brief Open device
   *
   * @return qint32 return code == 0 if no errors
   */
    qint32 open();

    /**
   * @brief Close device
   *
   */
    void close();

signals:

private slots:

private:
    QUsbDevicePrivate *const d_dummy;
    Q_DISABLE_COPY(QUsbDevice)

    quint16 m_timeout; /**< Device timeout */
    LogLevel m_log_level; /**< Log level */
    bool m_connected; /**< Connected boolean */
    Id m_id; /**< Device IDs */
    Config m_config; /**< Device config */
    DeviceSpeed m_spd; /**< Device speed */
};

QT_END_NAMESPACE

#endif // QUSBDEVICE_H
