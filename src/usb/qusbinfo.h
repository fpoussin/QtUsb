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
  QUsbDevice::IdList getPresentDevices(void) const;

  /**
   * @brief Check if device is present
   *
   * @param Id Device to search
   * @return bool true if present
   */
  bool isPresent(const QUsbDevice::Id &id) const;

signals:
  /**
   * @brief Signals a new device from the given list has been inserted
   *
   * @param IdList list of devices to match
   */
  void deviceInserted(QUsbDevice::IdList list);

  /**
   * @brief Signals a device from the given list has been removed
   *
   * @param IdList list of devices to match
   */
  void deviceRemoved(QUsbDevice::IdList list);

public slots:
  /**
   * @brief Add a device to the list
   *
   * @param Id device filter
   * @return bool false if device was already in the list, else true
   */
  bool addDevice(const QUsbDevice::Id &id);

  /**
   * @brief Remove a device from the list
   *
   * @param Id device filter
   * @return bool false if device was not in the list, else true
   */
  bool removeDevice(const QUsbDevice::Id &id);

  /**
   * @brief Search a device in a device list
   *
   * @param Id the device filter
   * @param IdList the device filter list
   * @return int index of the filter, returns -1 if not found
   */
  int findDevice(const QUsbDevice::Id &id,
                 const QUsbDevice::IdList &list) const;

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
  bool debug() const {return m_debug;}

protected slots:
  /**
   * @brief Checks for new events (insertions/removal) in the given list
   *
   * @param IdList list of devices to monitor
   */
  void monitorDevices(const QUsbDevice::IdList &list);

  void checkDevices();

protected:
  bool m_debug;
  QUsbDevice::IdList m_list;        /**< List of IDs we are using */
  QUsbDevice::IdList m_system_list; /**< List of all IDs in the system */

private:
  QUsbInfoPrivate * const d_dummy;
  Q_DISABLE_COPY(QUsbInfo)
};

QT_END_NAMESPACE

#endif // QUSBINFO_H
