#ifndef QUSBMANAGER_H
#define QUSBMANAGER_H

#include <QList>
#include <QThread>
#include <QUsb>

/**
 * @brief The QUsb manager class.
 * Handles USB events, searching, and devices.
 *
 */
class QUSBSHARED_EXPORT QUsbManager : public QThread {
  Q_OBJECT
public:
  /**
   * @brief Constructor
   *
   * @param parent
   */
  explicit QUsbManager(QObject *parent = 0);
  /**
   * @brief Destructor
   *
   */
  ~QUsbManager(void);

  /**
   * @brief Gets a list of devices present in the system
   * You have to add these devices to the list using addDevice beforehand
   *
   * @return QtUsb::FilterList
   */
  QtUsb::FilterList getPresentDevices(void);
  /**
   * @brief Check if device is present
   *
   * @param filter Device to search
   * @return bool true if present
   */
  bool isPresent(const QtUsb::DeviceFilter &filter);

signals:
  /**
   * @brief Signals a new device from the given list has been inserted
   *
   * @param filters list of devices to match
   */
  void deviceInserted(QtUsb::FilterList filters);
  /**
   * @brief Signals a device from the given list has been removed
   *
   * @param filters list of devices to match
   */
  void deviceRemoved(QtUsb::FilterList filters);

public slots:
  /**
   * @brief Add a device to the manager's list
   *
   * @param filter device filter
   * @return bool false if device was already in the list, else true
   */
  bool addDevice(const QtUsb::DeviceFilter &filter);
  /**
   * @brief Remove a device from the manager's list
   *
   * @param filter device filter
   * @return bool false if device was not in the list, else true
   */
  bool removeDevice(const QtUsb::DeviceFilter &filter);
  /**
   * @brief Search a device in a device list
   *
   * @param filter the device filter
   * @param list the device filter list
   * @return int index of the filter, returns -1 if not found
   */
  int findDevice(const QtUsb::DeviceFilter &filter,
                 const QtUsb::FilterList &list);

  /**
   * @brief
   *
   * @param dev QUsbDevice object pointer
   * @param filter Device filter
   * @param config Device configuration
   * @return QtUsb::DeviceStatus deviceOK on success
   */
  QtUsb::DeviceStatus openDevice(QUsbDevice *dev,
                                 const QtUsb::DeviceFilter &filter,
                                 const QtUsb::DeviceConfig &config);
  /**
   * @brief
   *
   * @param dev QUsbDevice object pointer
   * @return QtUsb::DeviceStatus deviceOK on success
   */
  QtUsb::DeviceStatus closeDevice(QUsbDevice *dev);

protected slots:
  /**
   * @brief Checks for new events (insertions/removal) in the given list
   *
   * @param list Lists of devices to monitor
   */
  void monitorDevices(const QtUsb::FilterList &list);
  /**
   * @brief
   *
   */
  void run(void);

protected:
  bool mStop; /**< Stop monitoring boolean */
  bool mHasHotplug;
  libusb_context *mCtx;                /**< libusb context */
  QList<QUsbDevice *> mUsedDeviceList; /**< List of devices in use */
  QtUsb::FilterList mFilterList;       /**< List of filters we are using */
  QtUsb::FilterList mSystemList;       /**< List of all filters in the system */
};

#endif // QUSBMANAGER_H
