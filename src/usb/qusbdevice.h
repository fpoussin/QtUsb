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

class Q_USB_EXPORT QUsbDevice : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QUsbDevice)

    friend QUsbTransfer;
    friend QUsbTransferPrivate;

public:
    static const quint16 DefaultTimeout = 250;

    typedef quint8 Endpoint;

    enum LogLevel {
        logNone = 0,
        logError = 1,
        logWarning = 2,
        logInfo = 3,
        logDebug = 4,
        logDebugAll = 5 // Includes libusb debug output
    };

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

    typedef struct Config {
        quint8 config;
        quint8 interface;
        quint8 alternate;

        bool operator==(const QUsbDevice::Config &o) const
        {
            return o.config == this->config && o.interface == this->interface && o.alternate == this->alternate;
        }

    } Config;

    typedef struct Id {
        quint16 pid;
        quint16 vid;

        bool operator==(const QUsbDevice::Id &o) const
        {
            return o.pid == this->pid && o.vid == this->vid;
        }

    } Id;

    typedef QList<Id> IdList;
    typedef QList<Config> ConfigList;

    enum DeviceSpeed {
        unknownSpeed = -1,
        lowSpeed = 0,
        fullSpeed,
        highSpeed,
        superSpeed,
        superSpeedPlus
    };
    Q_ENUM(DeviceSpeed)

    enum DeviceStatus {
        statusOK = 0,
        statusIoError = -1,
        statusInvalidParam = -2,
        statusAccessDenied = -3,
        statusNoSuchDevice = -4,
        statusNotFound = -5,
        statusBusy = -6,
        statusTimeout = -7,
        statusOverflow = -8,
        statusPipeError = -9,
        statusInterrupted = -10,
        statusNoMemory = -11,
        statusNotSupported = -12,
        StatusUnknownError = -99,
    };
    Q_ENUM(DeviceStatus)

    Q_PROPERTY(LogLevel logLevel READ logLevel WRITE setLogLevel)
    Q_PROPERTY(Id id READ id WRITE setId)
    Q_PROPERTY(Config config READ config WRITE setConfig)
    Q_PROPERTY(quint16 pid READ pid)
    Q_PROPERTY(quint16 vid READ vid)
    Q_PROPERTY(quint16 timeout READ timeout WRITE setTimeout)
    Q_PROPERTY(DeviceSpeed speed READ speed)

    explicit QUsbDevice(QObject *parent = Q_NULLPTR);
    ~QUsbDevice();

    void setLogLevel(LogLevel level);
    void setId(const Id &id);
    void setConfig(const Config &config);
    void setTimeout(quint16 timeout);
    Id id() const;
    Config config() const;
    bool isConnected() const;
    quint16 pid() const;
    quint16 vid() const;
    quint16 timeout() const;
    LogLevel logLevel() const;
    DeviceSpeed speed() const;
    QByteArray speedString() const;
    static IdList devices();

public slots:
    qint32 open();
    void close();

private:
    QUsbDevicePrivate *const d_dummy;
    Q_DISABLE_COPY(QUsbDevice)

    quint16 m_timeout;
    LogLevel m_log_level;
    bool m_connected;
    Id m_id;
    Config m_config;
    DeviceSpeed m_spd;
};

QT_END_NAMESPACE

#endif // QUSBDEVICE_H
