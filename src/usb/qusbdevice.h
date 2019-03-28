#ifndef QUSBDEVICE_H
#define QUSBDEVICE_H

#include "qusbglobal.h"
#include <QByteArray>
#include <QDebug>
#include <QString>

QT_BEGIN_NAMESPACE

class QUsbDevicePrivate;
class QUsbTransferHandler;
class QUsbTransferHandlerPrivate;

/**
 * @brief
 *
 */
class Q_USB_EXPORT QUsbDevice : public QObject
{
  Q_OBJECT
  Q_DECLARE_PRIVATE(QUsbDevice)

  friend QUsbTransferHandler;
  friend QUsbTransferHandlerPrivate;

public:

  static const quint16 DefaultTimeout = 250; /**< Default timeout in milliseconds */

  typedef quint8 Endpoint;

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
  Q_ENUM(DeviceSpeed)

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
  Q_ENUM(DeviceStatus)

  Q_PROPERTY(bool debug READ debug WRITE setDebug)
  Q_PROPERTY(DeviceFilter filter READ filter WRITE setFilter)
  Q_PROPERTY(DeviceConfig config READ config WRITE setConfig)
  Q_PROPERTY(quint16 pid READ pid)
  Q_PROPERTY(quint16 vid READ vid)
  Q_PROPERTY(quint16 timeout READ timeout WRITE setTimeout)
  Q_PROPERTY(DeviceSpeed speed READ speed)

  /**
   * @brief constructor
   *
   * @param parent
   */
  explicit QUsbDevice(QObject *parent = Q_NULLPTR);
  ~QUsbDevice();

  /**
   * @brief Enable/Disable debug
   *
   * @param enable
   */
  void setDebug(bool enable);
  /**
   * @brief Set device filter
   *
   * @param Filter Filter to apply
   */
  void setFilter(const DeviceFilter &filter) { m_filter = filter; }
  /**
   * @brief Set device config
   *
   * @param Config config to apply
   */
  void setConfig(const DeviceConfig &config) { m_config = config; }

  /**
   * @brief Set device timeout
   *
   * @param timeout Timeout in milliseconds
   */
  void setTimeout(quint16 timeout) { m_timeout = timeout; }

  /**
   * @brief Get current device filter
   *
   * @return DeviceFilter
   */
  DeviceFilter filter(void) const { return m_filter; }

  /**
   * @brief Get current device config
   *
   * @return DeviceConfig
   */
  DeviceConfig config(void) const { return m_config; }

  /**
   * @brief Get connection status
   *
   * @return bool
   */
  bool isConnected(void) const { return m_connected; }

  /**
   * @brief Get current device pid
   *
   * @return quint16
   */
  quint16 pid(void) const { return m_filter.pid; }
  /**
   * @brief Get current device vid
   *
   * @return quint16
   */
  quint16 vid(void) const { return m_filter.vid; }
  /**
   * @brief Get current timeout
   *
   * @return quint8
   */
  quint16 timeout(void) const { return m_timeout; }
  /**
   * @brief Get debug mode
   *
   * @return bool debug enabled
   */
  bool debug(void) const { return m_debug; }
  /**
   * @brief Get current device speed
   *
   * @return DeviceSpeed
   */
  DeviceSpeed speed(void) const { return m_spd; }
  /**
   * @brief Get current device speed string
   *
   * @return QByteArray
   */
  QByteArray speedString(void) const;

  /**
   * @brief Get a list of all USB devices available for use
   *
   * @return FilterList
   */
  static FilterList availableDevices(void);

public slots:
  /**
   * @brief Open device
   *
   * @return qint32 return code == 0 if no errors
   */
  qint32 open();

  /**
   * @brief Close device
   *
   */
  void close();

  /**
   * @brief Print settings to qDebug
   *
   */
  void showSettings(void);


signals:

private slots:

private:
  QUsbDevicePrivate * const d_dummy;
  Q_DISABLE_COPY(QUsbDevice)

  quint16 m_timeout;            /**< Device timeout */
  bool m_debug;                 /**< Debug enabled boolean */
  bool m_connected;             /**< Connected boolean */
  DeviceFilter m_filter; /**< Device filter */
  DeviceConfig m_config; /**< Device config */
  DeviceSpeed m_spd;     /**< Device speed */
};

QT_END_NAMESPACE

#endif // QUSBDEVICE_H
