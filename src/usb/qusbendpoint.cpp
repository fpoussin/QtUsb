#include "qusbendpoint_p.h"
#include "qusbdevice_p.h"

#include <QElapsedTimer>

#define DbgPrintError() qWarning("In %s, at %s:%d", Q_FUNC_INFO, __FILE__, __LINE__)
#define DbgPrintFuncName()                     \
    if (d->logLevel() >= QUsbDevice::logDebug) \
    qDebug() << "***[" << Q_FUNC_INFO << "]***"
#define DbgPrivPrintFuncName()                    \
    if (this->logLevel() >= QUsbDevice::logDebug) \
    qDebug() << "***[" << Q_FUNC_INFO << "]***"
#define DbgPrintCB(e)                          \
    if (e->logLevel() >= QUsbDevice::logDebug) \
    qDebug() << "***[" << Q_FUNC_INFO << "]***"

/* Write callback */
static void LIBUSB_CALL cb_out(struct libusb_transfer *transfer)
{
    QUsbEndpointPrivate *endpoint = reinterpret_cast<QUsbEndpointPrivate *>(transfer->user_data);
    DbgPrintCB(endpoint);

    libusb_transfer_status s = transfer->status;
    const int sent = transfer->actual_length;
    const int total = transfer->length;
    endpoint->setStatus(static_cast<QUsbEndpoint::Status>(s));

    if (endpoint->logLevel() >= QUsbDevice::logDebug)
        qDebug("OUT: status = %d, timeout = %d, endpoint = %x, actual_length = %d, length = %d",
               transfer->status,
               transfer->timeout,
               transfer->endpoint,
               transfer->actual_length,
               transfer->length);

    if (sent > 0) {
        endpoint->m_buf = endpoint->m_buf.mid(0, total - sent); // Remove what was sent
    }

    // Send remaining data
    if (total > sent) {
        transfer->buffer = reinterpret_cast<uchar *>(endpoint->m_buf.data()); // New data pointer
        transfer->length = endpoint->m_buf.size(); // New size
        libusb_submit_transfer(transfer);
        return;
    }

    libusb_free_transfer(transfer);
    endpoint->m_transfer = Q_NULLPTR;

    endpoint->m_buf_mutex.unlock();

    if (s != LIBUSB_TRANSFER_COMPLETED) {
        endpoint->error(static_cast<QUsbEndpoint::Status>(s));
    }
    if (sent > 0) {
        endpoint->bytesWritten(sent);
    }
}

/* Read callback */
static void LIBUSB_CALL cb_in(struct libusb_transfer *transfer)
{
    QUsbEndpointPrivate *endpoint = reinterpret_cast<QUsbEndpointPrivate *>(transfer->user_data);
    DbgPrintCB(endpoint);

    libusb_transfer_status s = transfer->status;
    const int received = transfer->actual_length;

    if (endpoint->logLevel() >= QUsbDevice::logDebug)
        qDebug("IN: status = %d, timeout = %d, endpoint = %x, actual_length = %d, length = %d",
               transfer->status,
               transfer->timeout,
               transfer->endpoint,
               transfer->actual_length,
               transfer->length);

    endpoint->setStatus(static_cast<QUsbEndpoint::Status>(s));
    if (s != LIBUSB_TRANSFER_COMPLETED) {
        endpoint->error(static_cast<QUsbEndpoint::Status>(s));
    } else {
        endpoint->m_buf_mutex.lock();
        const int previous_size = endpoint->m_buf.size();
        endpoint->m_buf.resize(previous_size + received);
        memcpy(endpoint->m_buf.data() + previous_size, transfer->buffer, static_cast<ulong>(received));
        endpoint->m_buf_mutex.unlock();
    }

    libusb_free_transfer(transfer);
    endpoint->m_transfer = Q_NULLPTR;

    endpoint->m_transfer_buf.clear(); // it's in fact transfer->buffer
    endpoint->m_transfer_mutex.unlock();

    if (received)
        endpoint->readyRead();

    // Start transfer over if polling is enabled
    if (endpoint->m_poll) {
        endpoint->readUsb(endpoint->m_poll_size);
    }
}

QUsbEndpointPrivate::QUsbEndpointPrivate()
    : m_poll(false), m_poll_size(1024), m_transfer(Q_NULLPTR)
{
}

void QUsbEndpointPrivate::readyRead()
{
    Q_Q(QUsbEndpoint);
    emit q->readyRead();
}

void QUsbEndpointPrivate::bytesWritten(qint64 bytes)
{
    Q_Q(QUsbEndpoint);
    emit q->bytesWritten(bytes);
}

void QUsbEndpointPrivate::error(QUsbEndpoint::Status error)
{
    Q_Q(QUsbEndpoint);
    emit q->error(error);
}

void QUsbEndpointPrivate::setStatus(QUsbEndpoint::Status status)
{
    Q_Q(QUsbEndpoint);
    DbgPrivPrintFuncName();

    q->m_status = status;
    switch (status) {
    case QUsbEndpoint::transferCompleted:
        q->setErrorString(QString::fromUtf8("transferCompleted"));
        break;
    case QUsbEndpoint::transferError:
        q->setErrorString(QString::fromUtf8("transferError"));
        break;
    case QUsbEndpoint::transferTimeout:
        q->setErrorString(QString::fromUtf8("transferTimeout"));
        break;
    case QUsbEndpoint::transferCanceled:
        q->setErrorString(QString::fromUtf8("transferCanceled"));
        break;
    case QUsbEndpoint::transferStall:
        q->setErrorString(QString::fromUtf8("transferStall"));
        break;
    case QUsbEndpoint::transferNoDevice:
        q->setErrorString(QString::fromUtf8("transferOverflow"));
        break;
    case QUsbEndpoint::transferOverflow:
        q->setErrorString(QString::fromUtf8("transferOverflow"));
        break;
    }
}

bool QUsbEndpointPrivate::isValid()
{
    Q_Q(QUsbEndpoint);
    DbgPrivPrintFuncName();
    return q->m_dev->d_func()->m_devHandle && q->m_dev->isConnected();
}

bool QUsbEndpointPrivate::prepareTransfer(libusb_transfer **tr, libusb_transfer_cb_fn cb, char *data, qint64 size, quint8 ep)
{
    Q_Q(QUsbEndpoint);
    DbgPrivPrintFuncName();

    auto buf = reinterpret_cast<uchar *>(data);
    auto maxSize = static_cast<int>(size);
    auto handle = q->m_dev->d_func()->m_devHandle;
    auto timeout = q->m_dev->timeout();

    if (q->m_type == QUsbEndpoint::bulkEndpoint) {
        *tr = libusb_alloc_transfer(0);
        libusb_fill_bulk_transfer(*tr,
                                  handle,
                                  ep,
                                  buf,
                                  maxSize,
                                  cb,
                                  this,
                                  timeout);
    } else if (q->m_type == QUsbEndpoint::interruptEndpoint) {
        *tr = libusb_alloc_transfer(0);
        libusb_fill_interrupt_transfer(*tr,
                                       handle,
                                       ep,
                                       buf,
                                       maxSize,
                                       cb,
                                       this,
                                       timeout);
    } else if (q->m_type == QUsbEndpoint::controlEndpoint) {
        *tr = libusb_alloc_transfer(0);
        libusb_fill_control_transfer(*tr,
                                     handle,
                                     buf,
                                     cb,
                                     this,
                                     timeout);
    } else if (q->m_type == QUsbEndpoint::isochronousEndpoint) { // Todo: Proper handling
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
            qWarning("QUsbEndpoint: Transfer buffer allocation failed");
        return false;
    }

    return true;
}

void QUsbEndpointPrivate::stopTransfer()
{
    DbgPrivPrintFuncName();
    // TODO: check if struct it not already freed
    // HINT: libusb_cancel_transfer is async, callback function is called, dont close device first on deconstruction...
    if (m_transfer != Q_NULLPTR)
        libusb_cancel_transfer(m_transfer);
}

int QUsbEndpointPrivate::readUsb(qint64 maxSize)
{
    Q_Q(QUsbEndpoint);
    DbgPrivPrintFuncName();
    int rc;

    // check it isn't closed already
    if (!q->m_dev->d_func()->m_devHandle || !q->m_dev->isConnected())
        return -1;

    if (maxSize == 0)
        return 0;

    m_transfer_mutex.lock();
    m_transfer_buf.resize(static_cast<int>(maxSize));
    if (!prepareTransfer(&m_transfer, cb_in, m_transfer_buf.data(), maxSize, q->m_ep))
        return -1;
    rc = libusb_submit_transfer(m_transfer);

    if (rc != LIBUSB_SUCCESS) {
        setStatus(QUsbEndpoint::transferError);
        error(QUsbEndpoint::transferError);
        // TODO: Check if QUsbEndpoint::QUsbDevice must be const...
        QUsbDevice *dev = const_cast<QUsbDevice *>(q->m_dev);
        dev->handleUsbError(rc);
        libusb_free_transfer(m_transfer);
        m_transfer = Q_NULLPTR;
        m_transfer_mutex.unlock();
        return rc;
    }

    return rc;
}

int QUsbEndpointPrivate::writeUsb(const char *data, qint64 maxSize)
{
    Q_Q(QUsbEndpoint);
    DbgPrivPrintFuncName();
    int rc;

    m_buf_mutex.lock();
    m_buf.resize(static_cast<int>(maxSize));
    memcpy(m_buf.data(), data, static_cast<ulong>(maxSize));

    if (!prepareTransfer(&m_transfer, cb_out, m_buf.data(), maxSize, q->m_ep))
        return -1;
    rc = libusb_submit_transfer(m_transfer);

    if (rc != LIBUSB_SUCCESS) {
        setStatus(QUsbEndpoint::transferError);
        error(QUsbEndpoint::transferError);
        // TODO: Check if QUsbEndpoint::QUsbDevice must be const...
        QUsbDevice *dev = const_cast<QUsbDevice *>(q->m_dev);
        dev->handleUsbError(rc);
        libusb_free_transfer(m_transfer);
        m_transfer = Q_NULLPTR;
        m_buf_mutex.unlock();
        return rc;
    }

    return rc;
}

void QUsbEndpointPrivate::setPolling(bool enable)
{
    Q_Q(QUsbEndpoint);
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

QUsbDevice::LogLevel QUsbEndpointPrivate::logLevel()
{
    Q_Q(QUsbEndpoint);
    return q->m_dev->logLevel();
}

/*!
    \class QUsbEndpoint

    \brief This class handles transfers between endpoints and the host.

    It works on top of libusb's async module.
    The QUsbDevice is set as parent object.
    You need one object per endpoint and direction.

    \reentrant
    \ingroup usb-main
    \inmodule QtUsb
 */

/*!
    \fn void QUsbEndpoint::error(Status error)
    \brief emits a signal on any transfer \a error.
 */

/*!
    \enum QUsbEndpoint::Status

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
    \enum QUsbEndpoint::Type

    \brief This enum describres the type of endpoint this object handles.

    \value controlEndpoint
    \value isochronousEndpoint
    \value bulkEndpoint
    \value interruptEndpoint
    \value streamEndpoint
 */

/*!
    \enum QUsbEndpoint::bmRequestType

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
    \enum QUsbEndpoint::bRequest

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
    \property QUsbEndpoint::type
    \brief Transfer type.
 */

/*!
    \property QUsbEndpoint::endpointIn
    \brief IN endpoint.
 */

/*!
    \property QUsbEndpoint::endpointOut
    \brief OUT endpoint.
 */

/*!
    \property QUsbEndpoint::polling
    \brief polling status.
 */

/*!
    \brief QUsbEndpoint constructor.

    This create an object of the given \a type for the \a ep endpoint.

    \a dev will be set as parent object.
 */
QUsbEndpoint::QUsbEndpoint(QUsbDevice *dev, QUsbEndpoint::Type type, quint8 ep)
    : QIODevice(*(new QUsbEndpointPrivate)), d_dummy(Q_NULLPTR), m_status(QUsbEndpoint::transferCanceled), m_dev(dev), m_type(type), m_ep(ep)
{
    Q_CHECK_PTR(dev);
    Q_D(QUsbEndpoint);
    DbgPrintFuncName();
    setParent(dev);
}

/*!
  \brief Will cancel all transfers on exit.
 */
QUsbEndpoint::~QUsbEndpoint()
{
    Q_D(QUsbEndpoint);
    DbgPrintFuncName();
    cancelTransfer();
}

/*!
    \brief Open the endpoint with \a mode.

    Returns \c true on success.
 */
bool QUsbEndpoint::open(QIODevice::OpenMode mode)
{
    Q_D(QUsbEndpoint);
    DbgPrintFuncName();

    switch (mode) {
    case QIODevice::ReadOnly:
    case QIODevice::WriteOnly:
        break;

    default: {
        qWarning("QUsbEndpoint::open Invalid mode");
        return false;
    }
    }

    bool b = QIODevice::open(mode);

    // Reset possible unclean mutex states.
    d->m_buf_mutex.tryLock();
    d->m_buf_mutex.unlock();
    d->m_buf_mutex.tryLock();
    d->m_buf_mutex.unlock();
    d->m_transfer_mutex.tryLock();
    d->m_transfer_mutex.unlock();

    // Set polling size to max packet size
    switch (m_type) {
    case bulkEndpoint:
        if (m_dev->speed() >= QUsbDevice::highSpeed)
            d->m_poll_size = 512;
        break;
    default:
        d->m_poll_size = 64;
    }

    if ((openMode() == ReadOnly && m_type == interruptEndpoint) || d->m_poll) {
        setPolling(true);
    }
    return b;
}

/*!
    \brief Close the transfer.

    This will cancel any ongoing transfers.
 */
void QUsbEndpoint::close()
{
    Q_D(QUsbEndpoint);
    DbgPrintFuncName();
    setPolling(false);
    QIODevice::close();

    // Wait for (canceled) transfers to finish
    while (d_func()->m_transfer != Q_NULLPTR)
        QThread::msleep(10);
}

/*!
    \brief Returns \c the transfer type.
 */
QUsbEndpoint::Type QUsbEndpoint::type() const
{
    return m_type;
}

/*!
    \brief Returns the transfer \c endpoint.
 */
quint8 QUsbEndpoint::endpoint() const
{
    return m_ep;
}

/*!
    \brief Always returns \c true.
 */
bool QUsbEndpoint::isSequential() const
{
    return true;
}

/*!
    \brief Get the endpoint \c status.
 */
QUsbEndpoint::Status QUsbEndpoint::status() const
{
    return m_status;
}

/*!
    \brief Bytes available to read.
 */
qint64 QUsbEndpoint::bytesAvailable() const
{
    return d_func()->m_buf.size() + QIODevice::bytesAvailable();
}

/*!
    \brief Bytes left to write.
 */
qint64 QUsbEndpoint::bytesToWrite() const
{
    return d_func()->m_buf.size() + QIODevice::bytesToWrite();
}

/*!
    \brief Wait for at least one byte to be written for \a msecs milliseconds.

    Returns \c true if any data was written before timeout.
 */
bool QUsbEndpoint::waitForBytesWritten(int msecs)
{
    Q_D(QUsbEndpoint);
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
bool QUsbEndpoint::waitForReadyRead(int msecs)
{
    Q_D(QUsbEndpoint);
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
void QUsbEndpoint::makeControlPacket(char *buffer, QUsbEndpoint::bmRequestType bmRequestType, QUsbEndpoint::bRequest bRequest, quint16 wValue, quint16 wIndex, quint16 wLength) const
{
    libusb_fill_control_setup(reinterpret_cast<uchar *>(buffer), bmRequestType, bRequest, wValue, wIndex, wLength);
}

/*!
    \brief \a enable or disable automating polling.
 */
void QUsbEndpoint::setPolling(bool enable)
{
    if (this->openMode() != ReadOnly)
        return;

    Q_D(QUsbEndpoint);
    DbgPrintFuncName();
    d->setPolling(enable);
}

/*!
    \brief Get polling status.

    return \c true if enabled.
 */
bool QUsbEndpoint::polling()
{
    Q_D(QUsbEndpoint);
    DbgPrintFuncName();
    return d->polling();
}

/*!
    \brief Manual IN (read) polling.

    return \c true if successful.
 */
bool QUsbEndpoint::poll()
{
    Q_D(QUsbEndpoint);
    DbgPrintFuncName();

    if (!isOpen()) {
        if (d->logLevel() >= QUsbDevice::logWarning)
            qWarning("QUsbEndpoint: Handle not open. Ignoring.");
        return false;
    }

    if (!(openMode() & ReadOnly)) {
        if (d->logLevel() >= QUsbDevice::logWarning)
            qWarning("QUsbEndpoint: Trying to poll without read mode. Ignoring.");
        return false;
    }

    if (polling()) {
        if (d->logLevel() >= QUsbDevice::logWarning)
            qWarning("QUsbEndpoint: Trying to poll with automatic polling enabled. Ignoring.");
        return false;
    }

    d->readUsb(d->m_poll_size);

    return true;
}

/*!

 */
void QUsbEndpoint::cancelTransfer()
{
    Q_D(QUsbEndpoint);
    d->stopTransfer();
}

/*!
    /reimp

    \brief Read \a maxSize bytes from the internal buffer to \a data.

    Returns \c read bytes.
 */
qint64 QUsbEndpoint::readData(char *data, qint64 maxSize)
{
    if (this->openMode() != ReadOnly)
        return -1;

    Q_D(QUsbEndpoint);
    DbgPrintFuncName();
    Q_CHECK_PTR(data);

    if (maxSize <= 0)
        return 0;

    QMutexLocker locker(&d->m_buf_mutex);
    qint64 read_size = d->m_buf.size();
    if (read_size == 0)
        return 0;
    if (!isOpen())
        return -1;
    if (read_size > maxSize)
        read_size = maxSize;

    memcpy(data, d->m_buf.data(), static_cast<ulong>(read_size));
    d->m_buf = d->m_buf.mid(static_cast<int>(read_size));

    return read_size;
}

/*!
    \brief Copies \a maxSize bytes from \a data to the internal write buffer and schedules an OUT transfer.

    Returns \c bytes written to the buffer.
 */
qint64 QUsbEndpoint::writeData(const char *data, qint64 maxSize)
{
    if (this->openMode() != WriteOnly)
        return -1;

    Q_D(QUsbEndpoint);
    DbgPrintFuncName();
    Q_CHECK_PTR(data);

    if (!d->isValid())
        return -1;

    if (d->writeUsb(data, maxSize) != 0)
        return -1;

    return maxSize;
}
