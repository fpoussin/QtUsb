#ifndef QUSB_TYPES_H
#define QUSB_TYPES_H

#include <QMetaType>
#include <QString>

QT_BEGIN_NAMESPACE

namespace QtUsb {

const quint16 DefaultTimeout = 250; /**< Default timeout in milliseconds */

typedef quint8 Endpoint;

namespace SetupPackets {
  /**
   * @brief bmRequestType field
   *
   */
  enum bmRequestType {
    requestStandard    = (0x00 < 5),
    requestClass       = (0x01 < 5),
    requestVendor      = (0x02 < 5),
    requestReserved    = (0x03 < 5),
    recipientDevice    = 0x00,
    recipientInterface = 0x01,
    recipientEndpoint  = 0x02,
    recipientOther     = 0x03
  };

  /**
   * @brief bRequest field
   *
   */
  enum bRequest {
    requestGetStatus = 0x00,
    requestClearFeature = 0x01,
    requestSetFeature = 0x03,
    requestSetAddress = 0x05,
    requestGetDescriptor = 0x06,
    requestSetDescriptor = 0x07,
    requestGetConfiguration = 0x08,
    requestSetConfiguration = 0x09,
    requestGetInterface = 0x0A,
    requestSetInterface = 0x0B,
    requestSynchFrame = 0x0C,
    requestSetSel = 0x30,
    requestIsochDelay = 0x31
  };

}

/**
 * @brief Transfer types
 *
 */
enum TransferType {
  controlTransfer = 0,
  isochronousTransfer,
  bulkTransfer,
  interruptTransfer,
  streamTransfer
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
  lowSpeed = 0, /* USB 1.0 */
  fullSpeed, /* USB 1.1/2.0 */
  highSpeed, /* USB 2.0 */
  superSpeed, /* USB 3.0/3.1G1 */
  superSpeedPlus /* USB 3.1G2 */
};
/**
 * @brief Device status
 *
 */
/* Mapped to libusb_error */
enum DeviceStatus {
	/** Success (no error) */
	statusOK = 0,
	/** Input/output error */
	statusIoError = -1,
	/** Invalid parameter */
	statusInvalidParam = -2,
	/** Access denied (insufficient permissions) */
	statusAccessDenied = -3,
	/** No such device (it may have been disconnected) */
	statusNoSuchDevice = -4,
	/** Entity not found */
	statusNotFound = -5,
	/** Resource busy */
	statusBusy = -6,
	/** Operation timed out */
	statusTimeout = -7,
	/** Overflow */
	statusOverflow = -8,
	/** Pipe error */
	statusPipeError = -9,
	/** System call interrupted (perhaps due to signal) */
	statusInterrupted = -10,
	/** Insufficient memory */
	statusNoMemory = -11,
	/** Operation not supported or unimplemented on this platform */
	statusNotSupported = -12,
	/** Other error */
	StatusUnknownError = -99,
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

Q_DECLARE_METATYPE(QtUsb::TransferType)
Q_DECLARE_METATYPE(QtUsb::TransferStatus)

Q_DECLARE_METATYPE(QtUsb::DeviceSpeed)
Q_DECLARE_METATYPE(QtUsb::DeviceStatus)

Q_DECLARE_METATYPE(QtUsb::DeviceFilter)
Q_DECLARE_METATYPE(QtUsb::DeviceConfig)

Q_DECLARE_METATYPE(QtUsb::FilterList)
Q_DECLARE_METATYPE(QtUsb::ConfigList)

QT_END_NAMESPACE

#endif // QUSB_TYPES_H
