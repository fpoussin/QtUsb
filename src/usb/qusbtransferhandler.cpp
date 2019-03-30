#include "qusbtransferhandler.h"
#include "qusbtransferhandler_p.h"
#include "qusbdevice_p.h"

#include <QElapsedTimer>

#define DbgPrintError() qWarning("In %s, at %s:%d", Q_FUNC_INFO, __FILE__, __LINE__)
#define DbgPrintFuncName() if (d->debug()) qDebug() << "***[" << Q_FUNC_INFO << "]***"
#define DbgPrintCB() if (handler->debug()) qDebug() << "***[" << Q_FUNC_INFO << "]***"

/* Write callback */
static void LIBUSB_CALL cb_out(struct libusb_transfer *transfer)
{
  QUsbTransferHandlerPrivate *handler = reinterpret_cast<QUsbTransferHandlerPrivate*>(transfer->user_data);
  DbgPrintCB();

  libusb_transfer_status s = transfer->status;
  const int sent = transfer->actual_length;
  const int total = transfer->length;
  handler->setStatus(static_cast<QUsbTransferHandler::Status>(s));

  if (handler->debug())
    qDebug("OUT: status = %d, timeout = %d, endpoint = %x, actual_length = %d, length = %d",
            transfer->status,
            transfer->timeout,
            transfer->endpoint,
            transfer->actual_length,
            transfer->length);

  if (sent > 0) {
    handler->m_write_buf = handler->m_write_buf.mid(0, total-sent); // Remove what was sent
  }

  // Send remaining data
  if (total > sent) {
    transfer->buffer = reinterpret_cast<uchar*>(handler->m_write_buf.data()); // New data pointer
    transfer->length = handler->m_write_buf.size(); // New size
    libusb_submit_transfer(transfer);
    return;
  }

  libusb_free_transfer(transfer);
  handler->m_transfer_out = Q_NULLPTR;

  handler->m_write_buf_mutex.unlock();

  if (s != LIBUSB_TRANSFER_COMPLETED) {
    handler->error(static_cast<QUsbTransferHandler::Status>(s));
  }
  if (sent > 0) {
    handler->bytesWritten(sent);
  }
}

/* Read callback */
static void LIBUSB_CALL cb_in(struct libusb_transfer *transfer)
{
  QUsbTransferHandlerPrivate *handler = reinterpret_cast<QUsbTransferHandlerPrivate*>(transfer->user_data);
  DbgPrintCB();

  libusb_transfer_status s = transfer->status;
  const int received = transfer->actual_length;

  if (handler->debug())
    qDebug("IN: status = %d, timeout = %d, endpoint = %x, actual_length = %d, length = %d",
            transfer->status,
            transfer->timeout,
            transfer->endpoint,
            transfer->actual_length,
            transfer->length);

  handler->setStatus(static_cast<QUsbTransferHandler::Status>(s));
  if (s != LIBUSB_TRANSFER_COMPLETED) {
    handler->error(static_cast<QUsbTransferHandler::Status>(s));
  }
  else {
    handler->m_read_buf_mutex.lock();
    const int previous_size = handler->m_read_buf.size();
    handler->m_read_buf.resize(previous_size + received);
    memcpy(handler->m_read_buf.data() + previous_size, transfer->buffer, static_cast<ulong>(received));
    handler->m_read_buf_mutex.unlock();
  }

  libusb_free_transfer(transfer);
  handler->m_transfer_in = Q_NULLPTR;

  handler->m_read_transfer_buf.clear(); // it's in fact transfer->buffer
  handler->m_read_transfer_mutex.unlock();

  if (received)
    handler->readyRead();

  // Start transfer over if polling is enabled
  if (handler->m_poll)
  {
    handler->readUsb(handler->m_poll_size);
  }
}

QUsbTransferHandlerPrivate::QUsbTransferHandlerPrivate() : m_poll(false), m_poll_size(64), m_transfer_in(Q_NULLPTR), m_transfer_out(Q_NULLPTR)
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

void QUsbTransferHandlerPrivate::error(QUsbTransferHandler::Status error)
{
  Q_Q(QUsbTransferHandler);
  emit q->error(error);
}

void QUsbTransferHandlerPrivate::setStatus(QUsbTransferHandler::Status status)
{
  Q_Q(QUsbTransferHandler);

  q->m_status = status;
  switch (status) {
    case QUsbTransferHandler::transferCompleted: q->setErrorString(QString::fromUtf8("transferCompleted")); break;
    case QUsbTransferHandler::transferError: q->setErrorString(QString::fromUtf8("transferError")); break;
    case QUsbTransferHandler::transferTimeout: q->setErrorString(QString::fromUtf8("transferTimeout")); break;
    case QUsbTransferHandler::transferCanceled: q->setErrorString(QString::fromUtf8("transferCanceled")); break;
    case QUsbTransferHandler::transferStall: q->setErrorString(QString::fromUtf8("transferStall")); break;
    case QUsbTransferHandler::transferNoDevice: q->setErrorString(QString::fromUtf8("transferOverflow")); break;
    case QUsbTransferHandler::transferOverflow: q->setErrorString(QString::fromUtf8("transferOverflow")); break;
  }
}

bool QUsbTransferHandlerPrivate::isValid()
{
  Q_Q(QUsbTransferHandler);
  return q->m_dev->d_func()->m_devHandle && q->m_dev->isConnected();
}

bool QUsbTransferHandlerPrivate::prepareTransfer(libusb_transfer **tr, libusb_transfer_cb_fn cb, char *data, qint64 size, QUsbDevice::Endpoint ep)
{
  Q_Q(QUsbTransferHandler);

  auto buf = reinterpret_cast<uchar*>(data);
  auto maxSize = static_cast<int>(size);
  auto handle = q->m_dev->d_func()->m_devHandle;
  auto timeout = q->m_dev->timeout();

  if (q->m_type == QUsbTransferHandler::bulkTransfer) {
    *tr = libusb_alloc_transfer(0);
    libusb_fill_bulk_transfer(*tr,
                              handle,
                              ep,
                              buf,
                              maxSize,
                              cb,
                              this,
                              timeout);
  }
  else if (q->m_type == QUsbTransferHandler::interruptTransfer) {
    *tr = libusb_alloc_transfer(0);
    libusb_fill_interrupt_transfer(*tr,
                                   handle,
                                   ep,
                                   buf,
                                   maxSize,
                                   cb,
                                   this,
                                   timeout);
  }
  else if (q->m_type == QUsbTransferHandler::controlTransfer) {
    *tr = libusb_alloc_transfer(0);
    libusb_fill_control_transfer(*tr,
                                 handle,
                                 buf,
                                 cb,
                                 this,
                                 timeout);
  }
  else if (q->m_type == QUsbTransferHandler::isochronousTransfer) { // Todo: Proper handling
    *tr = libusb_alloc_transfer(1);
    libusb_fill_iso_transfer(*tr,
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

  if (tr == Q_NULLPTR) {
    qWarning("QUsbTransferHandler: Transfer buffer allocation failed");
    return false;
  }

  return true;
}

void QUsbTransferHandlerPrivate::stopTransfer()
{
  if (m_transfer_in != Q_NULLPTR)
    libusb_cancel_transfer(m_transfer_in);
  if (m_transfer_out != Q_NULLPTR)
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
  if (!prepareTransfer(&m_transfer_in, cb_in, m_read_transfer_buf.data(), maxSize, q->m_in_ep)) return -1;
  rc = libusb_submit_transfer(m_transfer_in);

  if (rc != LIBUSB_SUCCESS) {
    q->m_dev->d_func()->printUsbError(rc);
    libusb_free_transfer(m_transfer_in);
    m_transfer_in = Q_NULLPTR;
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

  if (!prepareTransfer(&m_transfer_out, cb_out, m_write_buf.data(), maxSize, q->m_out_ep)) return -1;
  rc = libusb_submit_transfer(m_transfer_out);

  if (rc != LIBUSB_SUCCESS) {
    q->m_dev->d_func()->printUsbError(rc);
    libusb_free_transfer(m_transfer_out);
    m_transfer_out = Q_NULLPTR;
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

bool QUsbTransferHandlerPrivate::debug()
{
  Q_Q(QUsbTransferHandler);
  return q->m_dev->debug();
}

QUsbTransferHandler::QUsbTransferHandler(QUsbDevice *dev, QUsbTransferHandler::Type type, QUsbDevice::Endpoint in, QUsbDevice::Endpoint out) :
  QIODevice(*(new QUsbTransferHandlerPrivate)), d_dummy(Q_NULLPTR), m_status(QUsbTransferHandler::transferCanceled), m_dev(dev), m_type(type), m_in_ep(in), m_out_ep(out)
{
  Q_CHECK_PTR(dev);
  Q_D(QUsbTransferHandler);
  setParent(dev);

  // Set polling size to max packet size
  switch (type) {
  case bulkTransfer:
    if (m_dev->speed() >= QUsbDevice::highSpeed)
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
    if (openMode() & ReadOnly && m_type == interruptTransfer)
    {
      setPolling(true);
    }
    return b;
}

void QUsbTransferHandler::close()
{
  setPolling(false);
  QIODevice::close();

  // Wait for (canceled) transfers to finish
  while (d_func()->m_transfer_in != Q_NULLPTR || d_func()->m_transfer_out != Q_NULLPTR)
    QThread::msleep(10);
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

bool QUsbTransferHandler::poll()
{
  Q_D(QUsbTransferHandler);

  if (!isOpen())
  {
    qWarning("Handle not open. Ignoring.");
    return false;
  }

  if (!(openMode() & ReadOnly))
  {
    qWarning("QUsbTransferHandler: Trying to poll without read mode. Ignoring.");
    return false;
  }

  if (polling()) {
    qWarning("QUsbTransferHandler: Trying to poll with automatic polling enabled. Ignoring.");
    return false;
  }

  d->readUsb(d->m_poll_size);

  return true;
}

void QUsbTransferHandler::cancelTransfer()
{
  Q_D(QUsbTransferHandler);
  d->stopTransfer();
}

qint64 QUsbTransferHandler::readData(char *data, qint64 maxSize)
{
  Q_D(QUsbTransferHandler);
  DbgPrintFuncName();
  Q_CHECK_PTR(data);

  if (maxSize <= 0) return 0;

  QMutexLocker(&d->m_read_buf_mutex);
  qint64 read_size = d->m_read_buf.size();
  if (read_size == 0) return 0;
  if (!isOpen() && read_size == 0) return -1;
  if (read_size > maxSize) read_size = maxSize;

  memcpy(data, d->m_read_buf.data(), static_cast<ulong>(read_size));
  d->m_read_buf = d->m_read_buf.mid(static_cast<int>(read_size));

  return read_size;
}

qint64 QUsbTransferHandler::writeData(const char *data, qint64 maxSize)
{
  Q_D(QUsbTransferHandler);
  DbgPrintFuncName();
  Q_CHECK_PTR(data);

  if (!d->isValid()) return -1;

  return d->writeUsb(data, maxSize);
}
