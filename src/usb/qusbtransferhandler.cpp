#include "qusbtransferhandler.h"
#include "qusbtransferhandler_p.h"
#include "qusbdevice_p.h"
#include <QElapsedTimer>

#define UsbPrintError() qWarning("In %s, at %s:%d", Q_FUNC_INFO, __FILE__, __LINE__)
#define UsbPrintFuncName() if (m_dev->debug()) qDebug() << "***[" << Q_FUNC_INFO << "]***"

/* Write callback */
static void cb_out(struct libusb_transfer *transfer)
{
  QUsbTransferHandlerPrivate *handler = reinterpret_cast<QUsbTransferHandlerPrivate*>(transfer->user_data);

  handler->bytesWritten(transfer->actual_length);
  libusb_free_transfer(transfer);
}

/* Read callback */
static void cb_in(struct libusb_transfer *transfer)
{
  QUsbTransferHandlerPrivate *handler = reinterpret_cast<QUsbTransferHandlerPrivate*>(transfer->user_data);

  libusb_free_transfer(transfer);
  handler->readyRead();
}

QUsbTransferHandlerPrivate::QUsbTransferHandlerPrivate()
{
}

void QUsbTransferHandlerPrivate::readyRead()
{
  Q_Q(QUsbTransferHandler);
  emit q->readyRead();
}

void QUsbTransferHandlerPrivate::bytesWritten(qint64 bytes)
{
  Q_Q(QUsbTransferHandler);
  emit q->bytesWritten(bytes);
}

void QUsbTransferHandlerPrivate::prepareTransfer(libusb_transfer *tr, libusb_transfer_cb_fn cb, char *data, qint64 size, QtUsb::Endpoint ep)
{
  Q_Q(QUsbTransferHandler);

  auto buf = reinterpret_cast<uchar*>(data);
  auto maxSize = static_cast<int>(size);
  auto handle = q->m_dev->d_func()->m_devHandle;
  auto timeout = q->m_dev->timeout();


  if (q->m_type == QtUsb::bulkTransfer)
    libusb_fill_bulk_transfer(tr,
                              handle,
                              ep,
                              buf,
                              maxSize,
                              cb,
                              this,
                              timeout);
  else if (q->m_type == QtUsb::interruptTransfer)
    libusb_fill_interrupt_transfer(tr,
                                   handle,
                                   ep,
                                   buf,
                                   maxSize,
                                   cb,
                                   this,
                                   timeout);

  else if (q->m_type == QtUsb::isochronousTransfer)
    libusb_fill_iso_transfer(tr,
                             handle,
                             ep,
                             buf,
                             maxSize,
                             1,
                             cb_in,
                             this,
                             timeout);
}

QUsbTransferHandler::QUsbTransferHandler(QUsbDevice *dev, QtUsb::TransferType type, QtUsb::Endpoint out, QtUsb::Endpoint in, QObject* parent) :
  QIODevice(*(new QUsbTransferHandlerPrivate), parent), d_dummy(Q_NULLPTR), m_dev(dev), m_type(type), m_in_ep(in), m_out_ep(out)
{
  Q_CHECK_PTR(dev);
}

QUsbTransferHandler::~QUsbTransferHandler()
{

}

void QUsbTransferHandler::flush(quint8 endpoint) {
  QByteArray buf;
  int read_bytes;

  buf.resize(4096);
  libusb_bulk_transfer(m_dev->d_func()->m_devHandle,
                       endpoint,
                       reinterpret_cast<uchar*>(buf.data()),
                       buf.size(),
                       &read_bytes, 25);
}


qint64 QUsbTransferHandler::readData(char *data, qint64 maxSize)
{
  UsbPrintFuncName();
  Q_D(QUsbTransferHandler);
  Q_CHECK_PTR(data);
  qint32 rc;

  // check it isn't closed already
  if (!m_dev->d_func()->m_devHandle || !m_dev->isConnected()) return -1;

  if (maxSize == 0) return 0;

  d->m_transfer_in = libusb_alloc_transfer(0);
  d->prepareTransfer(d->m_transfer_in, cb_in, data, maxSize, m_in_ep);
  rc = libusb_submit_transfer(d->m_transfer_in);

  if (rc != LIBUSB_SUCCESS) {
    m_dev->d_func()->printUsbError(rc);
    return rc;
  }

  return rc;
}

qint64 QUsbTransferHandler::writeData(const char *data, qint64 maxSize)
{
  UsbPrintFuncName();
  Q_D(QUsbTransferHandler);
  Q_CHECK_PTR(data);
  qint32 rc;

  // check it isn't closed
  if (!m_dev->d_func()->m_devHandle || !m_dev->isConnected()) return -1;

  d->m_write_buf.resize(static_cast<int>(maxSize));
  memcpy(d->m_write_buf.data(), data, static_cast<ulong>(maxSize));

  d->m_transfer_in = libusb_alloc_transfer(0);
  d->prepareTransfer(d->m_transfer_in, cb_out, d->m_write_buf.data(), maxSize, m_out_ep);
  rc = libusb_submit_transfer(d->m_transfer_in);

  if (rc != LIBUSB_SUCCESS) {
    m_dev->d_func()->printUsbError(rc);
    return rc;
  }

  return rc;
}
