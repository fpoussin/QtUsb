#ifndef QUSBINFO_H
#define QUSBINFO_H

#include "qusbdevice.h"
#include <QList>

QT_BEGIN_NAMESPACE

class QUsbInfoPrivate;

/**
 * @brief The QtUsb info class.
 * Handles USB events and searching.
 * Can be used to monitor events for a list of devices or all system devices.
 */
class Q_USB_EXPORT QUsbInfo : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QUsbInfo)

public:
    /**
   * @brief Constructor
   *
   * @param parent
   */
    explicit QUsbInfo(QObject *parent = Q_NULLPTR);

    /**
   * @brief Destructor
   *
   */
    ~QUsbInfo(void);

    /**
   * @brief Gets a list of devices present in the system
   * You have to add these devices to the list using addDevice beforehand
   *
   * @return QUsbDevice::FilterList
   */
    QUsbDevice::IdList getPresentDevices(void) const;

    /**
   * @brief Check if device is present
   *
   * @param id device Id
   * @return bool true if present
   */
    bool isPresent(const QUsbDevice::Id &id) const;

signals:
    /**
   * @brief Signals a new device from the given list has been inserted
   *
   * @param list of devices to match
   */
    void deviceInserted(QUsbDevice::IdList list);

    /**
   * @brief Signals a device from the given list has been removed
   *
   * @param list of devices to match
   */
    void deviceRemoved(QUsbDevice::IdList list);

public slots:
    /**
   * @brief Add a device to the list
   *
   * @param id device Id
   * @return bool false if device was already in the list, else true
   */
    bool addDevice(const QUsbDevice::Id &id);

    /**
   * @brief Remove a device from the list
   *
   * @param id device Id
   * @return bool false if device was not in the list, else true
   */
    bool removeDevice(const QUsbDevice::Id &id);

    /**
   * @brief Search a device in a device list
   *
   * @param id the device filter
   * @param list the device filter list
   * @return int index of the filter, returns -1 if not found
   */
    int findDevice(const QUsbDevice::Id &id,
                   const QUsbDevice::IdList &list) const;

    /**
   * @brief set debug mode
   *
   * @param level debug level
   */
    void setLogLevel(QUsbDevice::LogLevel level);

    /**
   * @brief get debug mode
   *
   */
    QUsbDevice::LogLevel logLevel() const;

protected slots:
    /**
   * @brief Checks for new events (insertions/removal) in the given list
   *
   * @param list of devices to monitor
   */
    void monitorDevices(const QUsbDevice::IdList &list);

    /**
     * @brief Check if system device list has changed
     */
    void checkDevices();

protected:
    QUsbDevice::LogLevel m_log_level; /**< Log level */
    QUsbDevice::IdList m_list; /**< List of IDs we are using */
    QUsbDevice::IdList m_system_list; /**< List of all IDs in the system */

private:
    QUsbInfoPrivate *const d_dummy;
    Q_DISABLE_COPY(QUsbInfo)
};

QT_END_NAMESPACE

#endif // QUSBINFO_H
