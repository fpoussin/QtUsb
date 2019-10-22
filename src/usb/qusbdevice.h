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

    enum LogLevel : quint8 {
        logNone = 0,
        logError = 1,
        logWarning = 2,
        logInfo = 3,
        logDebug = 4,
        logDebugAll = 5
    };

    class Q_USB_EXPORT Config
    {
    public:
        quint8 config;
        quint8 interface;
        quint8 alternate;

        bool operator==(const QUsbDevice::Config &other) const;
        QUsbDevice::Config &operator=(const QUsbDevice::Config &other);
    };

    class Q_USB_EXPORT Id
    {
    public:
        quint16 pid;
        quint16 vid;
        quint8 bus;
        quint8 port;
        quint8 dClass;
        quint8 dSubClass;

        bool operator==(const QUsbDevice::Id &other) const;
        QUsbDevice::Id &operator=(const QUsbDevice::Id &other);
    };

    typedef QList<Id> IdList;
    typedef QList<Config> ConfigList;

    enum DeviceSpeed : qint8 {
        unknownSpeed = -1,
        lowSpeed = 0,
        fullSpeed,
        highSpeed,
        superSpeed,
        superSpeedPlus
    };
    Q_ENUM(DeviceSpeed)

    enum DeviceStatus : qint8 {
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
        statusUnknownError = -99,
    };
    Q_ENUM(DeviceStatus)

    Q_PROPERTY(LogLevel logLevel READ logLevel WRITE setLogLevel)
    Q_PROPERTY(Id id READ id WRITE setId)
    Q_PROPERTY(Config config READ config WRITE setConfig)
    Q_PROPERTY(quint16 pid READ pid)
    Q_PROPERTY(quint16 vid READ vid)
    Q_PROPERTY(quint16 timeout READ timeout WRITE setTimeout)
    Q_PROPERTY(DeviceSpeed speed READ speed)
    Q_PROPERTY(DeviceStatus status READ status NOTIFY statusChanged)
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectionChanged)

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
    DeviceStatus status() const;
    QByteArray statusString() const;
    static IdList devices();

private:
    void handleUsbError(int error_code);

signals:
    void statusChanged(QUsbDevice::DeviceStatus status);
    void connectionChanged(bool connected);

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
    DeviceStatus m_status;
};

Q_DECLARE_METATYPE(QUsbDevice::Config);
Q_DECLARE_METATYPE(QUsbDevice::Id);

QT_END_NAMESPACE

#endif // QUSBDEVICE_H
