#include "qusbtransfer.h"
#include "qusbtransfer_p.h"
#include "qusbdevice_p.h"

#include <QElapsedTimer>

#define DbgPrintError() qWarning("In %s, at %s:%d", Q_FUNC_INFO, __FILE__, __LINE__)
#define DbgPrintFuncName()                     \
    if (d->logLevel() >= QUsbDevice::logDebug) \
    qDebug() << "***[" << Q_FUNC_INFO << "]***"
#define DbgPrivPrintFuncName()                    \
    if (this->logLevel() >= QUsbDevice::logDebug) \
    qDebug() << "***[" << Q_FUNC_INFO << "]***"
#define DbgPrintCB()                                 \
    if (handler->logLevel() >= QUsbDevice::logDebug) \
    qDebug() << "***[" << Q_FUNC_INFO << "]***"

/* Write callback */
static void LIBUSB_CALL cb_out(struct libusb_transfer *transfer)
{
    QUsbTransferPrivate *handler = reinterpret_cast<QUsbTransferPrivate *>(transfer->user_data);
    DbgPrintCB();

    libusb_transfer_status s = transfer->status;
    const int sent = transfer->actual_length;
    const int total = transfer->length;
    handler->setStatus(static_cast<QUsbTransfer::Status>(s));

    if (handler->logLevel() >= QUsbDevice::logDebug)
        qDebug("OUT: status = %d, timeout = %d, endpoint = %x, actual_length = %d, length = %d",
               transfer->status,
               transfer->timeout,
               transfer->endpoint,
               transfer->actual_length,
               transfer->length);

    if (sent > 0) {
        handler->m_write_buf = handler->m_write_buf.mid(0, total - sent); // Remove what was sent
    }

    // Send remaining data
    if (total > sent) {
        transfer->buffer = reinterpret_cast<uchar *>(handler->m_write_buf.data()); // New data pointer
        transfer->length = handler->m_write_buf.size(); // New size
        libusb_submit_transfer(transfer);
        return;
    }

    libusb_free_transfer(transfer);
    handler->m_transfer_out = Q_NULLPTR;

    handler->m_write_buf_mutex.unlock();

    if (s != LIBUSB_TRANSFER_COMPLETED) {
        handler->error(static_cast<QUsbTransfer::Status>(s));
    }
    if (sent > 0) {
        handler->bytesWritten(sent);
    }
}

/* Read callback */
static void LIBUSB_CALL cb_in(struct libusb_transfer *transfer)
{
    QUsbTransferPrivate *handler = reinterpret_cast<QUsbTransferPrivate *>(transfer->user_data);
    DbgPrintCB();

    libusb_transfer_status s = transfer->status;
    const int received = transfer->actual_length;

    if (handler->logLevel() >= QUsbDevice::logDebug)
        qDebug("IN: status = %d, timeout = %d, endpoint = %x, actual_length = %d, length = %d",
               transfer->status,
               transfer->timeout,
               transfer->endpoint,
               transfer->actual_length,
               transfer->length);

    handler->setStatus(static_cast<QUsbTransfer::Status>(s));
    if (s != LIBUSB_TRANSFER_COMPLETED) {
        handler->error(static_cast<QUsbTransfer::Status>(s));
    } else {
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
    if (handler->m_poll) {
        handler->readUsb(handler->m_poll_size);
    }
}

QUsbTransferPrivate::QUsbTransferPrivate()
    : m_poll(false), m_poll_size(1024), m_transfer_in(Q_NULLPTR), m_transfer_out(Q_NULLPTR)
{
}

void QUsbTransferPrivate::readyRead()
{
    Q_Q(QUsbTransfer);
    emit q->readyRead();
}

void QUsbTransferPrivate::bytesWritten(qint64 bytes)
{
    Q_Q(QUsbTransfer);
    emit q->bytesWritten(bytes);
}

void QUsbTransferPrivate::error(QUsbTransfer::Status error)
{
    Q_Q(QUsbTransfer);
    emit q->error(error);
}

void QUsbTransferPrivate::setStatus(QUsbTransfer::Status status)
{
    Q_Q(QUsbTransfer);
    DbgPrivPrintFuncName();

    q->m_status = status;
    switch (status) {
    case QUsbTransfer::transferCompleted:
        q->setErrorString(QString::fromUtf8("transferCompleted"));
        break;
    case QUsbTransfer::transferError:
        q->setErrorString(QString::fromUtf8("transferError"));
        break;
    case QUsbTransfer::transferTimeout:
        q->setErrorString(QString::fromUtf8("transferTimeout"));
        break;
    case QUsbTransfer::transferCanceled:
        q->setErrorString(QString::fromUtf8("transferCanceled"));
        break;
    case QUsbTransfer::transferStall:
        q->setErrorString(QString::fromUtf8("transferStall"));
        break;
    case QUsbTransfer::transferNoDevice:
        q->setErrorString(QString::fromUtf8("transferOverflow"));
        break;
    case QUsbTransfer::transferOverflow:
        q->setErrorString(QString::fromUtf8("transferOverflow"));
        break;
    }
}

bool QUsbTransferPrivate::isValid()
{
    Q_Q(QUsbTransfer);
    DbgPrivPrintFuncName();
    return q->m_dev->d_func()->m_devHandle && q->m_dev->isConnected();
}

bool QUsbTransferPrivate::prepareTransfer(libusb_transfer **tr, libusb_transfer_cb_fn cb, char *data, qint64 size, QUsbDevice::Endpoint ep)
{
    Q_Q(QUsbTransfer);
    DbgPrivPrintFuncName();

    auto buf = reinterpret_cast<uchar *>(data);
    auto maxSize = static_cast<int>(size);
    auto handle = q->m_dev->d_func()->m_devHandle;
    auto timeout = q->m_dev->timeout();

    if (q->m_type == QUsbTransfer::bulkTransfer) {
        *tr = libusb_alloc_transfer(0);
        libusb_fill_bulk_transfer(*tr,
                                  handle,
                                  ep,
                                  buf,
                                  maxSize,
                                  cb,
                                  this,
                                  timeout);
    } else if (q->m_type == QUsbTransfer::interruptTransfer) {
        *tr = libusb_alloc_transfer(0);
        libusb_fill_interrupt_transfer(*tr,
                                       handle,
                                       ep,
                                       buf,
                                       maxSize,
                                       cb,
                                       this,
                                       timeout);
    } else if (q->m_type == QUsbTransfer::controlTransfer) {
        *tr = libusb_alloc_transfer(0);
        libusb_fill_control_transfer(*tr,
                                     handle,
                                     buf,
                                     cb,
                                     this,
                                     timeout);
    } else if (q->m_type == QUsbTransfer::isochronousTransfer) { // Todo: Proper handling
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
    } else {
        return false;
    }

    if (tr == Q_NULLPTR) {
        if (this->logLevel() >= QUsbDevice::logWarning)
            qWarning("QUsbTransferHandler: Transfer buffer allocation failed");
        return false;
    }

    return true;
}

void QUsbTransferPrivate::stopTransfer()
{
    DbgPrivPrintFuncName();
    // TODO: check if struct it not already freed
    // HINT: libusb_cancel_transfer is async, callback function is called, dont close device first on deconstruction...
    if (m_transfer_in != Q_NULLPTR)
        libusb_cancel_transfer(m_transfer_in);
    if (m_transfer_out != Q_NULLPTR)
        libusb_cancel_transfer(m_transfer_out);
}

int QUsbTransferPrivate::readUsb(qint64 maxSize)
{
    Q_Q(QUsbTransfer);
    DbgPrivPrintFuncName();
    int rc;

    // check it isn't closed already
    if (!q->m_dev->d_func()->m_devHandle || !q->m_dev->isConnected())
        return -1;

    if (maxSize == 0)
        return 0;

    m_read_transfer_mutex.lock();
    m_read_transfer_buf.resize(static_cast<int>(maxSize));
    if (!prepareTransfer(&m_transfer_in, cb_in, m_read_transfer_buf.data(), maxSize, q->m_in_ep))
        return -1;
    rc = libusb_submit_transfer(m_transfer_in);

    if (rc != LIBUSB_SUCCESS) {
        setStatus(QUsbTransfer::transferError);
        error(QUsbTransfer::transferError);
        // TODO: Check if QUsbTransfer::QUsbDevice must be const...
        QUsbDevice* dev = const_cast<QUsbDevice*>(q->m_dev);
        dev->handleUsbError(rc);
        libusb_free_transfer(m_transfer_in);
        m_transfer_in = Q_NULLPTR;
        m_read_transfer_mutex.unlock();
        return rc;
    }

    return rc;
}

int QUsbTransferPrivate::writeUsb(const char *data, qint64 maxSize)
{
    Q_Q(QUsbTransfer);
    DbgPrivPrintFuncName();
    int rc;

    m_write_buf_mutex.lock();
    m_write_buf.resize(static_cast<int>(maxSize));
    memcpy(m_write_buf.data(), data, static_cast<ulong>(maxSize));

    if (!prepareTransfer(&m_transfer_out, cb_out, m_write_buf.data(), maxSize, q->m_out_ep))
        return -1;
    rc = libusb_submit_transfer(m_transfer_out);

    if (rc != LIBUSB_SUCCESS) {
        setStatus(QUsbTransfer::transferError);
        error(QUsbTransfer::transferError);
        // TODO: Check if QUsbTransfer::QUsbDevice must be const...
        QUsbDevice* dev = const_cast<QUsbDevice*>(q->m_dev);
        dev->handleUsbError(rc);
        libusb_free_transfer(m_transfer_out);
        m_transfer_out = Q_NULLPTR;
        m_write_buf_mutex.unlock();
        return rc;
    }

    return rc;
}

void QUsbTransferPrivate::setPolling(bool enable)
{
    Q_Q(QUsbTransfer);
    DbgPrivPrintFuncName();
    m_poll = enable;

    if (enable) {
        // Start polling loop on IN if requirements are met
        if (q->openMode() & QIODevice::ReadOnly) {
            // Read once, loop will continue on its own as long as polling is enabled.
            this->readUsb(m_poll_size);
        }
    }
}

QUsbDevice::LogLevel QUsbTransferPrivate::logLevel()
{
    Q_Q(QUsbTransfer);
    return q->m_dev->logLevel();
}

/*!
    \class QUsbTransfer

    \brief This class handles transfers between endpoints and the host.

    It works on top of libusb's async module.
    The QUsbDevice is set as parent object.

    \reentrant
    \ingroup usb-main
    \inmodule QtUsb
 */

/*!
    \fn void QUsbTransfer::error(Status error)
    \brief emits a signal on any transfer \a error.
 */

/*!
    \enum QUsbTransfer::Status

    \brief This enum describres the last transfer status.

    \value transferCompleted    Transfer completed without errors. At least one byte transfered.
    \value transferError        Transfer completed with unknown error.
    \value transferTimeout      Transfer timed out.
    \value transferCanceled     Transfer canceled.
    \value transferStall        Transfer stalled (most likely hardware issue).
    \value transferNoDevice     Device not found.
    \value transferOverflow     More data received than requested.
 */

/*!
    \enum QUsbTransfer::Type

    \brief This enum describres the type of transfer this object handles.

    \value controlTransfer
    \value isochronousTransfer
    \value bulkTransfer
    \value interruptTransfer
    \value streamTransfer
 */

/*!
    \enum QUsbTransfer::bmRequestType

    \brief This enum describres a bmRequestType packet.

    \value requestStandard
    \value requestClass
    \value requestVendor
    \value requestReserved
    \value recipientDevice
    \value recipientInterface
    \value recipientEndpoint
    \value recipientOther
 */

/*!
    \enum QUsbTransfer::bRequest

    \brief This enum describres a bRequest packet.

    \value requestGetStatus
    \value requestClearFeature
    \value requestSetFeature
    \value requestSetAddress
    \value requestGetDescriptor
    \value requestSetDescriptor
    \value requestGetConfiguration
    \value requestSetConfiguration
    \value requestGetInterface
    \value requestSetInterface
    \value requestSynchFrame
    \value requestSetSel
    \value requestIsochDelay
 */

/*!
    \property QUsbTransfer::type
    \brief Transfer type.
 */

/*!
    \property QUsbTransfer::endpointIn
    \brief IN endpoint.
 */

/*!
    \property QUsbTransfer::endpointOut
    \brief OUT endpoint.
 */

/*!
    \property QUsbTransfer::polling
    \brief polling status.
 */

/*!
    \brief QUsbTransfer constructor.

    This create an object of the given transfer \a type, using endpoints \a in and \a out.

    \a dev will be set as parent object.
 */
QUsbTransfer::QUsbTransfer(QUsbDevice *dev, QUsbTransfer::Type type, QUsbDevice::Endpoint in, QUsbDevice::Endpoint out)
    : QIODevice(*(new QUsbTransferPrivate)), d_dummy(Q_NULLPTR), m_status(QUsbTransfer::transferCanceled), m_dev(dev), m_type(type), m_in_ep(in), m_out_ep(out)
{
    Q_CHECK_PTR(dev);
    Q_D(QUsbTransfer);
    DbgPrintFuncName();
    setParent(dev);
}

/*!
  \brief Will cancel all transfers on exit.
 */
QUsbTransfer::~QUsbTransfer()
{
    Q_D(QUsbTransfer);
    DbgPrintFuncName();
    cancelTransfer();
}

/*!
    \brief Open the transfer with \a mode.

    Returns \c true on success.
 */
bool QUsbTransfer::open(QIODevice::OpenMode mode)
{
    Q_D(QUsbTransfer);
    DbgPrintFuncName();
    bool b = QIODevice::open(mode);

    // Reset possible unclean mutex states.
    d->m_write_buf_mutex.tryLock();
    d->m_write_buf_mutex.unlock();
    d->m_read_buf_mutex.tryLock();
    d->m_read_buf_mutex.unlock();
    d->m_read_transfer_mutex.tryLock();
    d->m_read_transfer_mutex.unlock();

    // Set polling size to max packet size
    switch (m_type) {
    case bulkTransfer:
        if (m_dev->speed() >= QUsbDevice::highSpeed)
            d->m_poll_size = 512;
        break;
    default:
        d->m_poll_size = 64;
    }

    if ((openMode() & ReadOnly && m_type == interruptTransfer) || d->m_poll) {
        setPolling(true);
    }
    return b;
}

/*!
    \brief Close the transfer.

    This will cancel any ongoing transfers.
 */
void QUsbTransfer::close()
{
    Q_D(QUsbTransfer);
    DbgPrintFuncName();
    setPolling(false);
    QIODevice::close();

    // Wait for (canceled) transfers to finish
    while (d_func()->m_transfer_in != Q_NULLPTR || d_func()->m_transfer_out != Q_NULLPTR)
        QThread::msleep(10);
}

/*!
    \brief Returns \c the transfer type.
 */
QUsbTransfer::Type QUsbTransfer::type() const
{
    return m_type;
}

/*!
    \brief Returns the transfer \c IN endpoint.
 */
QUsbDevice::Endpoint QUsbTransfer::endpointIn() const
{
    return m_in_ep;
}

/*!
    \brief Returns \c the transfer OUT endpoint.
 */
QUsbDevice::Endpoint QUsbTransfer::endpointOut() const
{
    return m_out_ep;
}

/*!
    \brief Always returns \c true.
 */
bool QUsbTransfer::isSequential() const
{
    return true;
}

/*!
    \brief Get the transfer \c status.
 */
QUsbTransfer::Status QUsbTransfer::status() const
{
    return m_status;
}

/*!
    \brief Bytes available to read.
 */
qint64 QUsbTransfer::bytesAvailable() const
{
    return d_func()->m_read_buf.size() + QIODevice::bytesAvailable();
}

/*!
    \brief Bytes left to write.
 */
qint64 QUsbTransfer::bytesToWrite() const
{
    return d_func()->m_write_buf.size() + QIODevice::bytesToWrite();
}

/*!
    \brief Wait for at least one byte to be written for \a msecs milliseconds.

    Returns \c true if any data was written before timeout.
 */
bool QUsbTransfer::waitForBytesWritten(int msecs)
{
    Q_D(QUsbTransfer);
    DbgPrintFuncName();
    QElapsedTimer timer;
    timer.start();

    do {
        if (!this->bytesToWrite())
            return true;
    } while (timer.elapsed() < msecs);
    return false;
}

/*!
    \brief Wait for at least one byte to be available for \a msecs milliseconds.

    Returns \c true if any data was read before timeout.
 */
bool QUsbTransfer::waitForReadyRead(int msecs)
{
    Q_D(QUsbTransfer);
    DbgPrintFuncName();
    QElapsedTimer timer;
    timer.start();

    do {
        if (this->bytesAvailable())
            return true;
    } while (timer.elapsed() < msecs);
    return false;
}

/*!
    \brief Create a control packet using \a buffer, \a bmRequestType, \a bRequest, \a wValue, \a bRequest, \a wIndex, \a wLength.
 */
void QUsbTransfer::makeControlPacket(char *buffer, QUsbTransfer::bmRequestType bmRequestType, QUsbTransfer::bRequest bRequest, quint16 wValue, quint16 wIndex, quint16 wLength) const
{
    libusb_fill_control_setup(reinterpret_cast<uchar *>(buffer), bmRequestType, bRequest, wValue, wIndex, wLength);
}

/*!
    \brief \a enable or disable automating polling.
 */
void QUsbTransfer::setPolling(bool enable)
{
    Q_D(QUsbTransfer);
    DbgPrintFuncName();
    d->setPolling(enable);
}

/*!
    \brief Get polling status.

    return \c true if enabled.
 */
bool QUsbTransfer::polling()
{
    Q_D(QUsbTransfer);
    DbgPrintFuncName();
    return d->polling();
}

/*!
    \brief Manual IN (read) polling.

    return \c true if successful.
 */
bool QUsbTransfer::poll()
{
    Q_D(QUsbTransfer);
    DbgPrintFuncName();

    if (!isOpen()) {
        if (d->logLevel() >= QUsbDevice::logWarning)
            qWarning("QUsbTransferHandler: Handle not open. Ignoring.");
        return false;
    }

    if (!(openMode() & ReadOnly)) {
        if (d->logLevel() >= QUsbDevice::logWarning)
            qWarning("QUsbTransferHandler: Trying to poll without read mode. Ignoring.");
        return false;
    }

    if (polling()) {
        if (d->logLevel() >= QUsbDevice::logWarning)
            qWarning("QUsbTransferHandler: Trying to poll with automatic polling enabled. Ignoring.");
        return false;
    }

    d->readUsb(d->m_poll_size);

    return true;
}

/*!

 */
void QUsbTransfer::cancelTransfer()
{
    Q_D(QUsbTransfer);
    d->stopTransfer();
}

/*!
    /reimp

    \brief Read \a maxSize bytes from the internal buffer to \a data.

    Returns \c read bytes.
 */
qint64 QUsbTransfer::readData(char *data, qint64 maxSize)
{
    Q_D(QUsbTransfer);
    DbgPrintFuncName();
    Q_CHECK_PTR(data);

    if (maxSize <= 0)
        return 0;

    QMutexLocker(&d->m_read_buf_mutex);
    qint64 read_size = d->m_read_buf.size();
    if (read_size == 0)
        return 0;
    if (!isOpen())
        return -1;
    if (read_size > maxSize)
        read_size = maxSize;

    memcpy(data, d->m_read_buf.data(), static_cast<ulong>(read_size));
    d->m_read_buf = d->m_read_buf.mid(static_cast<int>(read_size));

    return read_size;
}

/*!
    \brief Copies \a maxSize bytes from \a data to the internal write buffer and schedules an OUT transfer.

    Returns \c bytes written to the buffer.
 */
qint64 QUsbTransfer::writeData(const char *data, qint64 maxSize)
{
    Q_D(QUsbTransfer);
    DbgPrintFuncName();
    Q_CHECK_PTR(data);

    if (!d->isValid())
        return -1;

    if (d->writeUsb(data, maxSize) != 0)
        return -1;

    return maxSize;
}
