#ifndef QUSBDEVICE_H
#define QUSBDEVICE_H

#include "qusbglobal.h"
#include "qusbtypes.h"
#include <QByteArray>
#include <QDebug>
#include <QString>
#include <QtEndian>

QT_BEGIN_NAMESPACE

class QUsbDevicePrivate;

/**
 * @brief
 *
 */
class Q_USB_EXPORT QUsbDevice : public QObject {
  Q_OBJECT

  Q_DECLARE_PRIVATE(QUsbDevice)

  Q_PROPERTY(bool debug READ debug WRITE setDebug)
  Q_PROPERTY(QtUsb::DeviceFilter filter READ filter WRITE setFilter)
  Q_PROPERTY(QtUsb::DeviceConfig config READ config WRITE setConfig)
  Q_PROPERTY(quint16 pid READ pid)
  Q_PROPERTY(quint16 vid READ vid)
  Q_PROPERTY(quint16 timeout READ timeout WRITE setTimeout)
  Q_PROPERTY(QtUsb::DeviceSpeed speed READ speed)

public:
  /**
   * @brief See base class
   *
   * @param parent
   */
  explicit QUsbDevice(QObject *parent = Q_NULLPTR);
  /**
   * @brief See base class
   *
   */
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
   * @brief See base class
   *
   * @return qint32
   */
  qint32 open();

  /**
   * @brief See base class
   *
   */
  void close();

  /**
   * @brief See base class
   *
   */
  void flush(quint8 endpoint);

  /**
   * @brief See base class
   *
   * @param buf
   * @param maxSize
   * @return qint32
   */
  qint32 read(QByteArray *buf, int len, QtUsb::endpoint_t endpoint);

  /**
   * @brief See base class
   *
   * @param buf
   * @param maxSize
   * @return qint32
   */
  qint32 write(const QByteArray *buf, int len, QtUsb::endpoint_t endpoint);

  /**
   * @brief Read maximum amount of bytes to buffer, up to 4096 bytes
   *
   * @param buf data to write into
   * @return qint32 actual number of bytes read on success, negative on error
   */
  qint32 read(QByteArray *buf, QtUsb::endpoint_t endpoint);

  /**
   * @brief Write full array to device
   *
   * @param buf data to write
   * @return qint32 actual number of bytes written on success, negative on error
   */
  qint32 write(const QByteArray &buf, QtUsb::endpoint_t endpoint);

  /**
   * @brief Write a single char
   *
   * @param c char
   * @return bool true on sucess
   */
  bool write(char c, QtUsb::endpoint_t endpoint);

  /**
   * @brief Read a single char
   *
   * @param c char
   * @return bool true on sucess
   */
  bool read(char *c, QtUsb::endpoint_t endpoint);

  /**
   * @brief Print settings to qDebug
   *
   */
  void showSettings(void);

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

  QByteArray m_read_buffer;
  int m_read_buffer_size;
};

QT_END_NAMESPACE

#endif // QUSBDEVICE_H
