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
  /**
   * @brief Transfer types
   *
   */
  enum Type {
    controlTransfer = 0,
    isochronousTransfer,
    bulkTransfer,
    interruptTransfer,
    streamTransfer
  };
  Q_ENUM(Type)

  /**
   * @brief Basically a copy of libusb's transfer enum
   *
   */
  enum Status {
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
  Q_ENUM(Status)

  Q_PROPERTY(Type type READ type)
  Q_PROPERTY(QUsbDevice::Endpoint endpointIn READ endpointIn)
  Q_PROPERTY(QUsbDevice::Endpoint endpointOut READ endpointOut)

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
                               Type type,
                               QUsbDevice::Endpoint in,
                               QUsbDevice::Endpoint out);

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
   * @return TransferType
   */
  Type type(void) const {return m_type;}

  /**
   * @brief get IN endpoint
   *
   * @return QUsbDevice::Endpoint
   */
  QUsbDevice::Endpoint endpointIn(void) const {return m_in_ep;}

  /**
   * @brief get OUT endpoint
   *
   * @return QUsbDevice::Endpoint
   */
  QUsbDevice::Endpoint endpointOut(void) const {return m_out_ep;}

  /**
   * @brief
   *
   * @return bool
   */
  bool isSequential() const {return true;}

   /**
    * @brief
    *
    * @return TransferStatus
    */
  Status status(void) const {return m_status;}

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
  bool poll();

public slots:
  /**
   * @brief Cancel ongoing async tranfer
   *
   */
  void cancelTransfer(void);

signals:
  void error(Status);

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

  Status m_status;
  const QUsbDevice *m_dev; /**< parent USB Device */
  const Type m_type; /**< Transfer type */
  const QUsbDevice::Endpoint m_in_ep; /**< IN endpoint */
  const QUsbDevice::Endpoint m_out_ep; /**< OUT endpoint */
};

#endif // QUSBTRANSFERHANDLER_H
