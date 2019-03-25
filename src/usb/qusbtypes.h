#ifndef QUSB_TYPES_H
#define QUSB_TYPES_H

#include <QMetaType>
#include <QString>

QT_BEGIN_NAMESPACE

namespace QtUsb {

const quint16 DefaultTimeout = 250; /**< Default timeout in milliseconds */

typedef quint8 Endpoint;

/**
 * @brief Transfer types
 *
 */
enum TransferType {
  controlTransfer = 0,
  bulkTransfer,
  interruptTransfer,
  isochronousTransfer
};

/**
 * @brief Basically a copy of libusb's transfer enum
 *
 */
enum TransferStatus {
        /** Transfer completed without error. Note that this does not indicate
        * that the entire amount of requested data was transferred. */
        transferCompleted,
        /** Transfer failed */
        transferError,
        /** Transfer timed out */
        transferTimeout,
        /** Transfer was cancelled */
        transferCanceled,
        /** For bulk/interrupt endpoints: halt condition detected (endpoint
         * stalled). For control endpoints: control request not supported. */
        transferStall,
        /** Device was disconnected */
        transferNoDevice,
        /** Device sent more data than requested */
        transferOverflow,
};

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
  QString guid;     /**< GUID (Windows) */

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
} // namespace QtUsb

Q_DECLARE_METATYPE(QtUsb::DeviceFilter)
Q_DECLARE_METATYPE(QtUsb::DeviceConfig)

Q_DECLARE_METATYPE(QtUsb::FilterList)
Q_DECLARE_METATYPE(QtUsb::ConfigList)

QT_END_NAMESPACE

#endif // QUSB_TYPES_H
