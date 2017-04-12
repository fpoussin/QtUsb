#ifndef QBASEUSB_H
#define QBASEUSB_H

#include "qusb_global.h"
#include "qusb_types.h"
#include <QDebug>
#include <QList>
#include <QString>

/**
 * @brief
 *
 */
class QUSBSHARED_EXPORT QBaseUsbDevice : public QObject {
  Q_OBJECT

public:
  /**
   * @brief Constructor
   *
   * @param parent
   */
  QBaseUsbDevice(QObject *parent = 0);
  /**
   * @brief Desctructor
   *
   */
  virtual ~QBaseUsbDevice();

  /**
   * @brief Enable/Disable debug
   *
   * @param enable
   */
  void setDebug(bool enable) { mDebug = enable; }
  /**
   * @brief Set device filter
   *
   * @param Filter Filter to apply
   */
  void setFilter(const QtUsb::DeviceFilter &filter) { mFilter = filter; }
  /**
   * @brief Set device config
   *
   * @param Config config to apply
   */
  void setConfig(const QtUsb::DeviceConfig &config) { mConfig = config; }

  /**
   * @brief Get current device filter
   *
   * @return QtUsb::DeviceFilter
   */
  QtUsb::DeviceFilter getFilter(void) { return mFilter; }
  /**
   * @brief Get current device config
   *
   * @return QtUsb::DeviceConfig
   */
  QtUsb::DeviceConfig getConfig(void) { return mConfig; }

  /**
   * @brief Get current device pid
   *
   * @return quint16
   */
  quint16 getPid(void) { return mFilter.pid; }
  /**
   * @brief Get current device vid
   *
   * @return quint16
   */
  quint16 getVid(void) { return mFilter.vid; }
  /**
   * @brief Get current read (IN) endpoint
   *
   * @return quint8
   */
  quint8 getReadEp(void) { return mConfig.readEp; }
  /**
   * @brief Get current write (OUT) endpoint
   *
   * @return quint8
   */
  quint8 getWriteEp(void) { return mConfig.writeEp; }
  /**
   * @brief Get current timeout
   *
   * @return quint16 Timeout
   */
  quint16 getTimeout(void) { return mTimeout; }

  /**
   * @brief Get current device speed
   *
   * @return QtUsb::DeviceSpeed
   */
  QtUsb::DeviceSpeed getSpeed(void) { return mSpd; }
  /**
   * @brief Get current device speed string
   *
   * @return QString
   */
  QString getSpeedString(void);

  /**
   * @brief Get a list of all USB devices available for use
   *
   * @return QtUsb::FilterList
   */
  static QtUsb::FilterList getAvailableDevices(void);

public slots:
  /**
   * @brief Open the devices
   *
   * @return qint32 0 on sucess, negative on error
   */
  virtual qint32 open() = 0;
  /**
   * @brief Close the device
   *
   */
  virtual void close() = 0;
  /**
   * @brief Flush the device buffer (IN only)
   *
   */
  virtual void flush() = 0;

  /**
   * @brief Write to device
   *
   * @param buf pointer to data
   * @param maxSize maximum number of bytes to write
   * @return qint32 actual number of bytes written on success, negative on error
   */
  virtual qint32 write(const QByteArray *buf, quint32 maxSize) = 0;
  /**
   * @brief Read from device
   *
   * @param buf pointer to data
   * @param maxSize maximum number of bytes to read
   * @return qint32 actual number of bytes read on success, negative on error
   */
  virtual qint32 read(QByteArray *buf, quint32 maxSize) = 0;

  /**
   * @brief Write full array to device
   *
   * @param buf data to write
   * @return qint32 actual number of bytes written on success, negative on error
   */
  qint32 write(const QByteArray &buf);
  /**
   * @brief Read maximum amount of bytes to buffer, up to 4096 bytes
   *
   * @param buf data to write into
   * @return qint32 actual number of bytes read on success, negative on error
   */
  qint32 read(QByteArray *buf);
  /**
   * @brief Write a single char
   *
   * @param c char
   * @return bool true on sucess
   */
  bool write(char c);
  /**
   * @brief Read a single char
   *
   * @param c char
   * @return bool true on sucess
   */
  bool read(char *c);

  /**
   * @brief Set device timeout
   *
   * @param timeout Timeout in milliseconds
   */
  void setTimeout(quint16 timeout);
  /**
   * @brief Print settings to qDebug
   *
   */
  void showSettings(void);

protected slots:
  /**
   * @brief Set default values (config)
   *
   */
  void setDefaults(void);

protected:
  QtUsb::DeviceFilter mFilter; /**< Device filter */
  QtUsb::DeviceConfig mConfig; /**< Device config */
  quint16 mTimeout;            /**< Device timeout */
  bool mDebug;                 /**< Debug enabled boolean */
  bool mConnected;             /**< Connected boolean */
  QtUsb::DeviceSpeed mSpd;     /**< Device speed */
};

#endif // QBASEUSB_H
