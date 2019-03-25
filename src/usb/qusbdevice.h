#ifndef QUSBDEVICE_H
#define QUSBDEVICE_H

#include "qusbglobal.h"
#include "qusbtypes.h"
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

  Q_PROPERTY(bool debug READ debug WRITE setDebug)
  Q_PROPERTY(QtUsb::DeviceFilter filter READ filter WRITE setFilter)
  Q_PROPERTY(QtUsb::DeviceConfig config READ config WRITE setConfig)
  Q_PROPERTY(quint16 pid READ pid)
  Q_PROPERTY(quint16 vid READ vid)
  Q_PROPERTY(quint16 timeout READ timeout WRITE setTimeout)
  Q_PROPERTY(QtUsb::DeviceSpeed speed READ speed)

public:
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
  void setFilter(const QtUsb::DeviceFilter &filter) { m_filter = filter; }
  /**
   * @brief Set device config
   *
   * @param Config config to apply
   */
  void setConfig(const QtUsb::DeviceConfig &config) { m_config = config; }

  /**
   * @brief Set device timeout
   *
   * @param timeout Timeout in milliseconds
   */
  void setTimeout(quint16 timeout) { m_timeout = timeout; }

  /**
   * @brief Get current device filter
   *
   * @return QtUsb::DeviceFilter
   */
  QtUsb::DeviceFilter filter(void) const { return m_filter; }

  /**
   * @brief Get current device config
   *
   * @return QtUsb::DeviceConfig
   */
  QtUsb::DeviceConfig config(void) const { return m_config; }

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
   * @brief Get current read (IN) endpoint
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
   * @return QtUsb::DeviceSpeed
   */
  QtUsb::DeviceSpeed speed(void) const { return m_spd; }
  /**
   * @brief Get current device speed string
   *
   * @return QByteArray
   */
  QByteArray speedString(void) const;

  /**
   * @brief Get a list of all USB devices available for use
   *
   * @return QtUsb::FilterList
   */
  static QtUsb::FilterList availableDevices(void);

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
  QtUsb::DeviceFilter m_filter; /**< Device filter */
  QtUsb::DeviceConfig m_config; /**< Device config */
  QtUsb::DeviceSpeed m_spd;     /**< Device speed */
};

QT_END_NAMESPACE

#endif // QUSBDEVICE_H
