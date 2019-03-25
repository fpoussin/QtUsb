#ifndef QUSBTRANSFERHANDLER_H
#define QUSBTRANSFERHANDLER_H

#include <QObject>
#include <QIODevice>
#include "qusbdevice.h"

class QUsbTransferHandlerPrivate;

class Q_USB_EXPORT QUsbTransferHandler : public QIODevice
{
  Q_OBJECT
  Q_DECLARE_PRIVATE(QUsbTransferHandler)

  friend QUsbDevice;
  friend QUsbDevicePrivate;

public:
  explicit QUsbTransferHandler(QUsbDevice * dev, QtUsb::TransferType type, QtUsb::Endpoint out, QtUsb::Endpoint in, QObject* parent = Q_NULLPTR);
  ~QUsbTransferHandler();

  /**
   * @brief See base class
   *
   */
  void flush(quint8 endpoint);

protected:
 qint64 readData(char* data, qint64 maxSize);
 qint64 writeData(const char* data, qint64 maxSize);

private:
  QUsbTransferHandlerPrivate * const d_dummy;
  Q_DISABLE_COPY(QUsbTransferHandler)

  const QUsbDevice * m_dev;
  const QtUsb::TransferType m_type;
  const QtUsb::Endpoint m_in_ep;
  const QtUsb::Endpoint m_out_ep;
};

#endif // QUSBTRANSFERHANDLER_H
