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

  handler->m_write_buf = handler->m_write_buf.mid(0, transfer->actual_length); // Remove what was sent
  handler->setStatus(static_cast<QtUsb::TransferStatus>(transfer->status));
  if (transfer->status != LIBUSB_TRANSFER_COMPLETED) {
    handler->error(static_cast<QtUsb::TransferStatus>(transfer->status));
  }
  else {
    handler->bytesWritten(transfer->actual_length);
  }

  handler->m_write_buf_mutex.unlock();
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
    handler->m_read_buf_mutex.lock();
    handler->m_read_buf.resize(transfer->actual_length);
    memcpy(handler->m_read_buf.data(), transfer->buffer, static_cast<ulong>(transfer->actual_length));
    handler->m_read_buf_mutex.unlock();
    handler->readyRead();
  }

  handler->m_read_transfer_mutex.unlock();
  libusb_free_transfer(transfer);

  if (handler->polling())
  {
    handler->readUsb(handler->m_poll_size);
  }
}

QUsbTransferHandlerPrivate::QUsbTransferHandlerPrivate()
{
  m_poll_size = 64;
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

bool QUsbTransferHandlerPrivate::isValid()
{
  Q_Q(QUsbTransferHandler);
  return q->m_dev->d_func()->m_devHandle && q->m_dev->isConnected();
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
  else if (q->m_type == QtUsb::isochronousTransfer) { // Todo: Proper handling
    tr = libusb_alloc_transfer(1);
    libusb_fill_iso_transfer(tr,
                             handle,
                             ep,
                             buf,
                             maxSize,
                             1,
                             cb,
                             this,
                             timeout);
  }
  else {
    return false;
  }

  return true;
}

void QUsbTransferHandlerPrivate::stopTransfer()
{
  libusb_cancel_transfer(m_transfer_in);
  libusb_cancel_transfer(m_transfer_out);
}

int QUsbTransferHandlerPrivate::readUsb(qint64 maxSize)
{
  Q_Q(QUsbTransferHandler);
  int rc;

  // check it isn't closed already
  if (!q->m_dev->d_func()->m_devHandle || !q->m_dev->isConnected()) return -1;

  if (maxSize == 0) return 0;

  m_read_transfer_mutex.lock();
  m_read_transfer_buf.resize(static_cast<int>(maxSize));
  if (!prepareTransfer(m_transfer_in, cb_in, m_read_transfer_buf.data(), maxSize, q->m_in_ep)) return -1;
  rc = libusb_submit_transfer(m_transfer_in);

  if (rc != LIBUSB_SUCCESS) {
    q->m_dev->d_func()->printUsbError(rc);
    m_read_transfer_mutex.unlock();
    return rc;
  }

  return rc;
}

int QUsbTransferHandlerPrivate::writeUsb(const char *data, qint64 maxSize)
{
  Q_Q(QUsbTransferHandler);
  int rc;

  m_write_buf_mutex.lock();
  m_write_buf.resize(static_cast<int>(maxSize));
  memcpy(m_write_buf.data(), data, static_cast<ulong>(maxSize));

  if (!prepareTransfer(m_transfer_out, cb_out, m_write_buf.data(), maxSize, q->m_out_ep)) return -1;
  rc = libusb_submit_transfer(m_transfer_out);

  if (rc != LIBUSB_SUCCESS) {
    q->m_dev->d_func()->printUsbError(rc);
    m_write_buf_mutex.unlock();
    return rc;
  }

  return rc;
}

void QUsbTransferHandlerPrivate::setPolling(bool enable)
{
  Q_Q(QUsbTransferHandler);
  m_poll = enable;

  if (enable)
  {
    // Start polling loop on IN if requirements are met
    if (q->openMode() & QIODevice::ReadOnly)
    {
      // Read once, loop will continue on its own as long as polling is enabled.
      this->readUsb(m_poll_size);
    }
  }
}

QUsbTransferHandler::QUsbTransferHandler(QUsbDevice *dev, QtUsb::TransferType type, QtUsb::Endpoint in, QtUsb::Endpoint out) :
  QIODevice(*(new QUsbTransferHandlerPrivate)), d_dummy(Q_NULLPTR), m_status(QtUsb::transferCanceled), m_dev(dev), m_type(type), m_in_ep(in), m_out_ep(out)
{
  Q_CHECK_PTR(dev);
  Q_D(QUsbTransferHandler);
  setParent(dev);

  // Set polling size to max packet size
  switch (type) {
  case QtUsb::bulkTransfer:
    if (m_dev->speed() >= QtUsb::highSpeed)
      d->m_poll_size = 512;
    break;
  default:
    d->m_poll_size = 64;
  }
}

QUsbTransferHandler::~QUsbTransferHandler()
{
    cancelTransfer();
}

bool QUsbTransferHandler::open(QIODevice::OpenMode mode)
{
    bool b = QIODevice::open(mode);
    if (openMode() & ReadOnly && m_type == QtUsb::interruptTransfer)
    {
      setPolling(true);
    }
    return b;
}

void QUsbTransferHandler::close()
{
  QIODevice::close();
}

qint64 QUsbTransferHandler::bytesAvailable() const
{
  return d_func()->m_read_buf.size() + QIODevice::bytesAvailable();
}

qint64 QUsbTransferHandler::bytesToWrite() const
{
  return d_func()->m_write_buf.size() + QIODevice::bytesToWrite();
}

bool QUsbTransferHandler::waitForBytesWritten(int msecs)
{
    QElapsedTimer timer;
    timer.start();

    do {
      if (!this->bytesToWrite()) return true;
    } while (timer.elapsed() < msecs);
    return false;
}

bool QUsbTransferHandler::waitForReadyRead(int msecs)
{
    QElapsedTimer timer;
    timer.start();

    do {
      if (this->bytesAvailable()) return true;
    } while (timer.elapsed() < msecs);
    return false;
}

void QUsbTransferHandler::makeControlPacket(char *buffer, quint8 bmRequestType, quint8 bRequest, quint16 wValue, quint16 wIndex, quint16 wLength) const
{
  libusb_fill_control_setup(reinterpret_cast<uchar*>(buffer), bmRequestType, bRequest, wValue, wIndex, wLength);
}

void QUsbTransferHandler::setPolling(bool enable)
{
  Q_D(QUsbTransferHandler);
  d->setPolling(enable);
}

bool QUsbTransferHandler::polling()
{
  Q_D(QUsbTransferHandler);
  return d->polling();
}

void QUsbTransferHandler::poll()
{
  Q_D(QUsbTransferHandler);

  if (!(openMode() & ReadOnly))
  {
    qWarning("QUsbTransferHandler: Trying to poll without read mode. Ignoring.");
    return;
  }

  if (!polling()) {
    // Do nothing if auto polling is enabled
    d->readUsb(d->m_poll_size);
  }
  else {
    qWarning("QUsbTransferHandler: Trying to poll with automatic polling enabled. Ignoring.");
  }
}

void QUsbTransferHandler::cancelTransfer()
{
  Q_D(QUsbTransferHandler);
  d->stopTransfer();
}

qint64 QUsbTransferHandler::readData(char *data, qint64 maxSize)
{
  UsbPrintFuncName();
  Q_D(QUsbTransferHandler);
  Q_CHECK_PTR(data);

  if (maxSize <= 0) return 0;

  QMutexLocker(&d->m_read_buf_mutex);
  qint64 buf_size = d->m_read_buf.size();
  if (buf_size == 0) return 0;
  if (!isOpen() && buf_size == 0) return -1;
  if (buf_size > maxSize) buf_size = maxSize;

  memcpy(data, d->m_read_buf.data(), static_cast<ulong>(buf_size));
  d->m_read_buf = d->m_read_buf.mid(0, static_cast<int>(buf_size));

  return buf_size;
}

qint64 QUsbTransferHandler::writeData(const char *data, qint64 maxSize)
{
  UsbPrintFuncName();
  Q_D(QUsbTransferHandler);
  Q_CHECK_PTR(data);

  if (!d->isValid()) return -1;

  return d->writeUsb(data, maxSize);
}
