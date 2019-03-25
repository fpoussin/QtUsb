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
   * @brief
   *
   * @param dev parent USB Device
   * @param type Transfer type for this object
   * @param in IN Endpoint
   * @param out OUT endpoint
   * @param parent
   */
  explicit QUsbTransferHandler(QUsbDevice * dev, QtUsb::TransferType type, QtUsb::Endpoint in, QtUsb::Endpoint out, QObject* parent = Q_NULLPTR);
  /**
   * @brief
   *
   */
  ~QUsbTransferHandler();

  Q_PROPERTY(QtUsb::TransferType type READ type)
  Q_PROPERTY(QtUsb::Endpoint endpointIn READ endpointIn)
  Q_PROPERTY(QtUsb::Endpoint endpointOut READ endpointOut)

  /**
   * @brief get transfer type
   *
   * @return QtUsb::TransferType
   */
  QtUsb::TransferType type(void) {return m_type;}
  /**
   * @brief get IN endpoint
   *
   * @return QtUsb::Endpoint
   */
  QtUsb::Endpoint endpointIn(void) {return m_in_ep;}
  /**
   * @brief get OUT endpoint
   *
   * @return QtUsb::Endpoint
   */
  QtUsb::Endpoint endpointOut(void) {return m_out_ep;}

  bool busy(void) {return m_busy;}

  /**
   * @brief flush IN endpoint
   *
   */
  void flush();

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

  bool m_busy;
  const QUsbDevice * m_dev; /**< parent USB Device */
  const QtUsb::TransferType m_type; /**< Transfer type */
  const QtUsb::Endpoint m_in_ep; /**< IN endpoint */
  const QtUsb::Endpoint m_out_ep; /**< OUT endpoint */
};

#endif // QUSBTRANSFERHANDLER_H
