#include "qusbtransferhandler.h"
#include "qusbtransferhandler_p.h"
#include "qusbdevice_p.h"

#define UsbPrintError() qWarning("In %s, at %s:%d", Q_FUNC_INFO, __FILE__, __LINE__)
#define UsbPrintFuncName() if (m_dev->debug()) qDebug() << "***[" << Q_FUNC_INFO << "]***"

/* Write callback */
static void cb_out(struct libusb_transfer *transfer)
{
  QUsbTransferHandlerPrivate *handler = reinterpret_cast<QUsbTransferHandlerPrivate*>(transfer->user_data);

  handler->setStatus(static_cast<QtUsb::TransferStatus>(transfer->status));
  if (transfer->status != LIBUSB_TRANSFER_COMPLETED) {
    handler->error(static_cast<QtUsb::TransferStatus>(transfer->status));
  }
  else {
    handler->bytesWritten(transfer->actual_length);
  }

  handler->m_mutex.unlock();
  libusb_free_transfer(transfer);
}

/* Read callback */
static void cb_in(struct libusb_transfer *transfer)
{
  QUsbTransferHandlerPrivate *handler = reinterpret_cast<QUsbTransferHandlerPrivate*>(transfer->user_data);

  handler->setStatus(static_cast<QtUsb::TransferStatus>(transfer->status));
  if (transfer->status != LIBUSB_TRANSFER_COMPLETED) {
    handler->error(static_cast<QtUsb::TransferStatus>(transfer->status));
  }
  else {
    handler->readyRead();
  }

  handler->m_mutex.unlock();
  libusb_free_transfer(transfer);
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

void QUsbTransferHandlerPrivate::error(QtUsb::TransferStatus error)
{
  Q_Q(QUsbTransferHandler);
  emit q->error(error);
}

void QUsbTransferHandlerPrivate::setStatus(QtUsb::TransferStatus status)
{
  Q_Q(QUsbTransferHandler);

  q->m_status = status;
  switch (status) {
    case QtUsb::transferCompleted: q->setErrorString(QString::fromUtf8("transferCompleted")); break;
    case QtUsb::transferError: q->setErrorString(QString::fromUtf8("transferError")); break;
    case QtUsb::transferTimeout: q->setErrorString(QString::fromUtf8("transferTimeout")); break;
    case QtUsb::transferCanceled: q->setErrorString(QString::fromUtf8("transferCanceled")); break;
    case QtUsb::transferStall: q->setErrorString(QString::fromUtf8("transferStall")); break;
    case QtUsb::transferNoDevice: q->setErrorString(QString::fromUtf8("transferOverflow")); break;
    case QtUsb::transferOverflow: q->setErrorString(QString::fromUtf8("transferOverflow")); break;
  }
}

bool QUsbTransferHandlerPrivate::prepareTransfer(libusb_transfer *tr, libusb_transfer_cb_fn cb, char *data, qint64 size, QtUsb::Endpoint ep)
{
  Q_Q(QUsbTransferHandler);

  auto buf = reinterpret_cast<uchar*>(data);
  auto maxSize = static_cast<int>(size);
  auto handle = q->m_dev->d_func()->m_devHandle;
  auto timeout = q->m_dev->timeout();

  if (q->m_type == QtUsb::bulkTransfer) {
    tr = libusb_alloc_transfer(0);
    libusb_fill_bulk_transfer(tr,
                              handle,
                              ep,
                              buf,
                              maxSize,
                              cb,
                              this,
                              timeout);
  }
  else if (q->m_type == QtUsb::interruptTransfer) {
    tr = libusb_alloc_transfer(0);
    libusb_fill_interrupt_transfer(tr,
                                   handle,
                                   ep,
                                   buf,
                                   maxSize,
                                   cb,
                                   this,
                                   timeout);
  }
  else if (q->m_type == QtUsb::controlTransfer) {
    tr = libusb_alloc_transfer(0);
    libusb_fill_control_transfer(tr,
                                 handle,
                                 buf,
                                 cb,
                                 this,
                                 timeout);
  }
  else if (q->m_type == QtUsb::isochronousTransfer) {
    tr = libusb_alloc_transfer(1);
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
  else {
    return false;
  }

  return true;
}

QUsbTransferHandler::QUsbTransferHandler(QUsbDevice *dev, QtUsb::TransferType type, QtUsb::Endpoint in, QtUsb::Endpoint out) :
  QIODevice(*(new QUsbTransferHandlerPrivate)), d_dummy(Q_NULLPTR), m_status(QtUsb::transferCanceled), m_dev(dev), m_type(type), m_in_ep(in), m_out_ep(out)
{
  Q_CHECK_PTR(dev);
  setParent(dev);
}

QUsbTransferHandler::~QUsbTransferHandler()
{

}

void QUsbTransferHandler::flush() {
  Q_D(QUsbTransferHandler);
  QByteArray buf;
  int read_bytes;

  // check it isn't closed already
  if (!m_dev->d_func()->m_devHandle || !m_dev->isConnected()) return;

  QMutexLocker(&d->m_mutex);
  buf.resize(4096);
  libusb_bulk_transfer(m_dev->d_func()->m_devHandle,
                       m_in_ep,
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

  d->m_mutex.lock();
  if (!d->prepareTransfer(d->m_transfer_in, cb_in, data, maxSize, m_in_ep)) return -1;
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

  d->m_mutex.lock();
  d->m_write_buf.resize(static_cast<int>(maxSize));
  memcpy(d->m_write_buf.data(), data, static_cast<ulong>(maxSize));

  if (!d->prepareTransfer(d->m_transfer_out, cb_out, d->m_write_buf.data(), maxSize, m_out_ep)) return -1;
  rc = libusb_submit_transfer(d->m_transfer_out);

  if (rc != LIBUSB_SUCCESS) {
    m_dev->d_func()->printUsbError(rc);
    return rc;
  }

  return rc;
}
