#ifndef QUSBINFO_H
#define QUSBINFO_H

#include "qusbdevice.h"
#include <QList>

QT_BEGIN_NAMESPACE

/**
 * @brief The QUsb info class.
 * Handles USB events, searching, and devices.
 *
 */

class QUsbInfoPrivate;

class Q_USB_EXPORT QUsbInfo : public QObject {
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
  QUsbDevice::FilterList getPresentDevices(void);

  /**
   * @brief Check if device is present
   *
   * @param filter Device to search
   * @return bool true if present
   */
  bool isPresent(const QUsbDevice::DeviceFilter &filter);

signals:
  /**
   * @brief Signals a new device from the given list has been inserted
   *
   * @param filters list of devices to match
   */
  void deviceInserted(QUsbDevice::FilterList filters);

  /**
   * @brief Signals a device from the given list has been removed
   *
   * @param filters list of devices to match
   */
  void deviceRemoved(QUsbDevice::FilterList filters);

public slots:
  /**
   * @brief Add a device to the list
   *
   * @param filter device filter
   * @return bool false if device was already in the list, else true
   */
  bool addDevice(const QUsbDevice::DeviceFilter &filter);

  /**
   * @brief Remove a device from the list
   *
   * @param filter device filter
   * @return bool false if device was not in the list, else true
   */
  bool removeDevice(const QUsbDevice::DeviceFilter &filter);

  /**
   * @brief Search a device in a device list
   *
   * @param filter the device filter
   * @param list the device filter list
   * @return int index of the filter, returns -1 if not found
   */
  int findDevice(const QUsbDevice::DeviceFilter &filter,
                 const QUsbDevice::FilterList &list);

  /**
   * @brief set debug mode
   *
   * @param debug
   */
  void setDebug(bool debug);

  /**
   * @brief get debug mode
   *
   */
  bool debug() {return m_debug;}

  /**
   * @brief
   *
   * @param dev QUsbDevice object pointer
   * @param filter Device filter
   * @param config Device configuration
   * @return QUsbDevice::DeviceStatus deviceOK on success
   */
  QUsbDevice::DeviceStatus openDevice(QUsbDevice *dev,
                                 const QUsbDevice::DeviceFilter &filter,
                                 const QUsbDevice::DeviceConfig &config);

  /**
   * @brief
   *
   * @param dev QUsbDevice object pointer
   * @return QUsbDevice::DeviceStatus deviceOK on success
   */
  QUsbDevice::DeviceStatus closeDevice(QUsbDevice *dev);

protected slots:
  /**
   * @brief Checks for new events (insertions/removal) in the given list
   *
   * @param list Lists of devices to monitor
   */
  void monitorDevices(const QUsbDevice::FilterList &list);

  void checkDevices();

protected:
  bool m_debug;
  QList<QUsbDevice *> m_used_device_list; /**< List of devices in use */
  QUsbDevice::FilterList m_filter_list;        /**< List of filters we are using */
  QUsbDevice::FilterList m_system_list; /**< List of all filters in the system */

private:
  QUsbInfoPrivate * const d_dummy;
  Q_DISABLE_COPY(QUsbInfo)
};

QT_END_NAMESPACE

#endif // QUSBINFO_H
