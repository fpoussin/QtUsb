#ifndef QUSB_H
#define QUSB_H

#include "qusbglobal.h"
#include <QObject>
#include <QList>

// Stupid windows conflict
#ifdef interface
  #undef interface
#endif

QT_BEGIN_NAMESPACE

class QUsbPrivate;

class Q_USB_EXPORT QUsb : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QUsb)

    Q_PROPERTY(QUsb::LogLevel logLevel READ logLevel WRITE setLogLevel)

public:

    enum LogLevel : quint8 {
        logNone = 0,
        logError = 1,
        logWarning = 2,
        logInfo = 3,
        logDebug = 4,
        logDebugAll = 5
    };
    Q_ENUM(LogLevel);

    enum DeviceSpeed : qint8 {
        unknownSpeed = -1,
        lowSpeed = 0,
        fullSpeed,
        highSpeed,
        superSpeed,
        superSpeedPlus
    };
    Q_ENUM(DeviceSpeed);

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
    Q_ENUM(DeviceStatus);

    class Q_USB_EXPORT Config
    {
    public:
        Config(quint8 _config = 1, qint8 _interface = -1, quint8 _alternate = 0);
        Config(const QUsb::Config &other);
        bool operator==(const QUsb::Config &other) const;
        QUsb::Config &operator=(QUsb::Config other);
        operator QString() const;

        quint8 config;
        qint8  interface;
        quint8 alternate;
    };
    typedef QList<Config> DeviceConfigurations;
    class Q_USB_EXPORT EndpointDescription // build out ability to describe endpoint
    {
    public:
        EndpointDescription(quint8 address = 0, quint8 attributes = 0, quint8 config = 0, quint8 interface = 0, quint8 alternate = 0);
        EndpointDescription(const QUsb::EndpointDescription &other);
        bool operator==(const QUsb::EndpointDescription &other) const;
        QUsb::EndpointDescription &operator=(QUsb::EndpointDescription other);
        operator QString() const;
        quint8 address;
        quint8 attributes;
        quint8 config;
        quint8 interface;
        quint8 alternate;
    };
    typedef QList<EndpointDescription> EndpointDescriptionList;

    class Q_USB_EXPORT Id
    {
    public:
        Id(quint16 _pid = 0, quint16 _vid = 0, quint8 _bus = busAny, quint8 _port = portAny, quint8 _class = 0, quint8 _subclass = 0);
        Id(const QUsb::Id &other);
        bool operator==(const QUsb::Id &other) const;
        QUsb::Id &operator=(QUsb::Id other);
        operator QString() const;

        quint16 pid;
        quint16 vid;
        quint8 bus;
        quint8 port;
        quint8 dClass;
        quint8 dSubClass;
        quint8 configCount;
        QString description;
        DeviceConfigurations configurations;
        QUsb::EndpointDescriptionList endpoints;
    };



    typedef QList<Id> IdList;
    typedef QList<Config> ConfigList;

    enum Bus : quint8 {
        busAny = 255,
    };

    enum Port : quint8 {
        portAny = 255,
    };

    explicit QUsb(QObject *parent = Q_NULLPTR);
    ~QUsb(void);

    static IdList devices();
    static int xusb(const Id &id, const Config &config);
    bool isPresent(const Id &id) const;
    int findDevice(const Id &id,
                   const IdList &list) const;
    void setLogLevel(LogLevel level);
    LogLevel logLevel() const;

Q_SIGNALS:
    void deviceInserted(Id id);
    void deviceRemoved(Id id);

public slots:
    bool addDevice(const Id &id);
    bool removeDevice(const Id &id);

protected slots:
    void monitorDevices(const IdList &list);
    void checkDevices();

protected:
    LogLevel m_log_level; /*!< Log level */
    IdList m_list; /*!< List of IDs we are using */
    IdList m_system_list; /*!< List of all IDs in the system */

private:
    QUsbPrivate *const d_dummy;
    Q_DISABLE_COPY(QUsb)
};

Q_DECLARE_METATYPE(QUsb::EndpointDescription);
Q_DECLARE_METATYPE(QUsb::Config);
Q_DECLARE_METATYPE(QUsb::Id);

QT_END_NAMESPACE

#endif // QUSB_H
