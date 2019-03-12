#ifndef QUSB_TYPES_H
#define QUSB_TYPES_H

#include <QMetaType>

namespace QtUsb {

const quint16 DefaultTimeout = 250; /**< Default timeout in milliseconds */
                                    /**
                                     * @brief USB speeds
                                     *
                                     */
enum DeviceSpeed {
  unknownSpeed = -1,
  lowSpeed = 0,
  fullSpeed,
  highSpeed,
  superSpeed
};
/**
 * @brief Device status
 *
 */
enum DeviceStatus {
  deviceOK = 0,
  deviceBusy = -1,
  deviceNotFound = -2,
  devicePgmError = -3
};

/**
 * @brief Device configuration
 *
 */
typedef struct {
  quint8 readEp;    /**< Read Endpoint (IN) */
  quint8 writeEp;   /**< Write Endpoint (OUT) */
  quint8 config;    /**< Configuration index */
  quint8 interface; /**< Interface index */
  quint8 alternate; /**< Alternate configuration index */

} DeviceConfig;

/**
 * @brief Device filter
 *
 */
typedef struct {
  quint16 pid;      /**< Product ID */
  quint16 vid;      /**< Vendor ID */
  DeviceConfig cfg; /**< Configuration for a given device */

} DeviceFilter;

/**
 * @brief List of device filters
 *
 */
typedef QList<DeviceFilter> FilterList;
/**
 * @brief List of device configs
 *
 */
typedef QList<DeviceConfig> ConfigList;
}

Q_DECLARE_METATYPE(QtUsb::DeviceFilter)
Q_DECLARE_METATYPE(QtUsb::DeviceConfig)

Q_DECLARE_METATYPE(QtUsb::FilterList)
Q_DECLARE_METATYPE(QtUsb::ConfigList)

#endif // QUSB_TYPES_H
