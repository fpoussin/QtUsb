#ifndef QLIBUSB_H
#define QLIBUSB_H

#include "qbaseusb_p.h"
#include "qusbcompat_p.h"
#include <QByteArray>
#include <QDebug>
#include <QString>
#include <QtEndian>
#include <libusb-1.0/libusb.h>

QT_BEGIN_NAMESPACE

/**
 * @brief
 *
 */
class Q_USB_EXPORT QUsbDevice : public QBaseUsbDevice {
  Q_OBJECT

public:
  /**
   * @brief See base class
   *
   * @param parent
   */
  explicit QUsbDevice(QBaseUsbDevice *parent = Q_NULLPTR);
  /**
   * @brief See base class
   *
   */
  ~QUsbDevice();

  /**
   * @brief See base class
   *
   * @return QtUsb::FilterList
   */
  static QtUsb::FilterList getAvailableDevices(void);

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
  void flush();
  /**
   * @brief See base class
   *
   * @param buf
   * @param maxSize
   * @return qint32
   */
  qint32 read(QByteArray *buf, quint32 len);
  /**
   * @brief See base class
   *
   * @param buf
   * @param maxSize
   * @return qint32
   */
  qint32 write(const QByteArray *buf, quint32 len);

  /**
   * @brief See base class
   *
   * @param enable
   */
  void setDebug(bool enable);

private slots:

private:
  /**
   * @brief Print error code to qWarning
   *
   * @param error_code
   */
  void printUsbError(int error_code);

  libusb_device **mDevs;            /**< libusb device ptr to ptr */
  libusb_device_handle *mDevHandle; /**< libusb device handle ptr */
  libusb_context *mCtx;             /**< libusb context */
  QByteArray mReadBuffer;
  quint32 mReadBufferSize;
};

QT_END_NAMESPACE

#endif // QLIBUSB_H
