#ifndef QUSBTRANSFERHANDLER_H
#define QUSBTRANSFERHANDLER_H

#include <QObject>
#include <QIODevice>
#include "qusbdevice.h"

class QUsbTransferHandlerPrivate;

/**
 * @brief
 *
 */
class Q_USB_EXPORT QUsbTransferHandler : public QIODevice
{
  Q_OBJECT
  Q_DECLARE_PRIVATE(QUsbTransferHandler)

public:
  Q_PROPERTY(QtUsb::TransferType type READ type)
  Q_PROPERTY(QtUsb::Endpoint endpointIn READ endpointIn)
  Q_PROPERTY(QtUsb::Endpoint endpointOut READ endpointOut)

  /**
   * @brief
   *
   * @param dev parent USB Device
   * @param type Transfer type for this object
   * @param in IN Endpoint
   * @param out OUT endpoint
   * @param parent
   */
  explicit QUsbTransferHandler(QUsbDevice * dev,
                               QtUsb::TransferType type,
                               QtUsb::Endpoint in,
                               QtUsb::Endpoint out);

  /**
   * @brief
   *
   */
  ~QUsbTransferHandler();

  /**
   * @brief
   *
   * @param mode
   * @return bool
   */
  bool open(QIODevice::OpenMode mode);

  /**
   * @brief
   *
   */

  void close();

  /**
   * @brief get transfer type
   *
   * @return QtUsb::TransferType
   */
  QtUsb::TransferType type(void) const {return m_type;}

  /**
   * @brief get IN endpoint
   *
   * @return QtUsb::Endpoint
   */
  QtUsb::Endpoint endpointIn(void) const {return m_in_ep;}

  /**
   * @brief get OUT endpoint
   *
   * @return QtUsb::Endpoint
   */
  QtUsb::Endpoint endpointOut(void) const {return m_out_ep;}

  /**
   * @brief
   *
   * @return bool
   */
  bool isSequential() const {return true;}

   /**
    * @brief
    *
    * @return QtUsb::TransferStatus
    */
  QtUsb::TransferStatus status(void) const {return m_status;}

  /**
   * @brief
   *
   * @return qint64
   */
  qint64 bytesAvailable() const;

  /**
   * @brief
   *
   * @return qint64
   */
  qint64 bytesToWrite() const;

  /**
   * @brief
   *
   * @param msecs
   * @return bool
   */
  bool waitForBytesWritten(int msecs);
  /**
   * @brief
   *
   * @param msecs
   * @return bool
   */
  bool waitForReadyRead(int msecs);

  /**
   * @brief Populate buffer with a control packet
   *
   * @param buffer
   * @param bmRequestType
   * @param bRequest
   * @param wValue
   * @param wIndex
   * @param wLength
   */
  void makeControlPacket(char * 	buffer,
                         quint8 	bmRequestType,
                         quint8 	bRequest,
                         quint16	wValue,
                         quint16	wIndex,
                         quint16	wLength ) const;

  /**
   * @brief Enable automatic IN polling (recuring read read)
   * This is enabled by default on interrupt endpoints
   *
   * @param enable
   */
  void setPolling(bool enable);

  /**
   * @brief Polling status
   *
   * @return bool
   */
  bool polling();

  /**
   * @brief Poll IN endpoint for data
   *
   */
  void poll();

public slots:
  /**
   * @brief Cancel ongoing async tranfer
   *
   */
  void cancelTransfer(void);

signals:
  void error(QtUsb::TransferStatus);

protected:
 /**
  * @brief
  *
  * @param data
  * @param maxSize
  * @return qint64
  */
 qint64 readData(char* data, qint64 maxSize);
 /**
  * @brief
  *
  * @param data
  * @param maxSize
  * @return qint64
  */
 qint64 writeData(const char* data, qint64 maxSize);

private:
  QUsbTransferHandlerPrivate * const d_dummy;
  /**
   * @brief
   *
   * @param
   */
  Q_DISABLE_COPY(QUsbTransferHandler)

  QtUsb::TransferStatus m_status;
  const QUsbDevice *m_dev; /**< parent USB Device */
  const QtUsb::TransferType m_type; /**< Transfer type */
  const QtUsb::Endpoint m_in_ep; /**< IN endpoint */
  const QtUsb::Endpoint m_out_ep; /**< OUT endpoint */
};

#endif // QUSBTRANSFERHANDLER_H
