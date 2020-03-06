#ifndef QUSBINFO_H
#define QUSBINFO_H

#include "qusbdevice.h"
#include <QList>

QT_BEGIN_NAMESPACE

class QUsbInfoPrivate;

class Q_USB_EXPORT QUsbInfo : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QUsbInfo)

    Q_PROPERTY(QUsbDevice::LogLevel logLevel READ logLevel WRITE setLogLevel)

public:
    explicit QUsbInfo(QObject *parent = Q_NULLPTR);
    ~QUsbInfo(void);

    static QUsbDevice::IdList devices();
    bool isPresent(const QUsbDevice::Id &id) const;
    int findDevice(const QUsbDevice::Id &id,
                   const QUsbDevice::IdList &list) const;
    void setLogLevel(QUsbDevice::LogLevel level);
    QUsbDevice::LogLevel logLevel() const;

Q_SIGNALS:
    void deviceInserted(QUsbDevice::Id id);
    void deviceRemoved(QUsbDevice::Id id);

public slots:
    bool addDevice(const QUsbDevice::Id &id);
    bool removeDevice(const QUsbDevice::Id &id);

protected slots:
    void monitorDevices(const QUsbDevice::IdList &list);
    void checkDevices();

protected:
    QUsbDevice::LogLevel m_log_level; /*!< Log level */
    QUsbDevice::IdList m_list; /*!< List of IDs we are using */
    QUsbDevice::IdList m_system_list; /*!< List of all IDs in the system */

private:
    QUsbInfoPrivate *const d_dummy;
    Q_DISABLE_COPY(QUsbInfo)
};

QT_END_NAMESPACE

#endif // QUSBINFO_H
