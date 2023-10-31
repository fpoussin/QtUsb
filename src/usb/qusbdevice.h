#ifndef QUSBDEVICE_H
#define QUSBDEVICE_H

#include "QtUsb/qusbglobal.h"
#include "QtUsb/qusb.h"
#include <QByteArray>
#include <QDebug>
#include <QString>

QT_BEGIN_NAMESPACE

class QUsbDevicePrivate;
class QUsbEndpoint;
class QUsbEndpointPrivate;

class Q_USB_EXPORT QUsbDevice : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QUsbDevice)

    friend QUsbEndpoint;
    friend QUsbEndpointPrivate;

public:
    static const quint16 DefaultTimeout = 250;

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

    Q_PROPERTY(QUsb::LogLevel logLevel READ logLevel WRITE setLogLevel)
    Q_PROPERTY(QUsb::Id id READ id WRITE setId)
    Q_PROPERTY(QUsb::Config config READ config WRITE setConfig)
    Q_PROPERTY(quint16 pid READ pid)
    Q_PROPERTY(quint16 vid READ vid)
    Q_PROPERTY(quint16 timeout READ timeout WRITE setTimeout)
    Q_PROPERTY(DeviceSpeed speed READ speed)
    Q_PROPERTY(DeviceStatus status READ status NOTIFY statusChanged)
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectionChanged)

    explicit QUsbDevice(QObject *parent = Q_NULLPTR);
    ~QUsbDevice();

    void setLogLevel(QUsb::LogLevel level);
    void setId(const QUsb::Id &id);
    void setConfig(const QUsb::Config &config);
    void setTimeout(quint16 timeout);
    bool isConnected() const;
    quint16 pid() const;
    quint16 vid() const;
    quint16 timeout() const;
    QUsb::LogLevel logLevel() const;
    DeviceSpeed speed() const;
    QByteArray speedString() const;
    DeviceStatus status() const;
    QByteArray statusString() const;

    QUsb::Id id() const;
    QUsb::Config config() const;

private:
    void handleUsbError(int error_code);

Q_SIGNALS:
    void statusChanged(QUsbDevice::DeviceStatus status);
    void connectionChanged(bool connected);

public Q_SLOTS:
    qint32 open();
    void close();

private:
    QUsbDevicePrivate *const d_dummy;
    Q_DISABLE_COPY(QUsbDevice)

    quint16 m_timeout;
    QUsb::LogLevel m_log_level;
    bool m_connected;
    QUsb::Id m_id;
    QUsb::Config m_config;
    DeviceSpeed m_spd;
    DeviceStatus m_status;
};

QT_END_NAMESPACE

#endif // QUSBDEVICE_H
