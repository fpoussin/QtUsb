#ifndef QUSBTRANSFER_H
#define QUSBTRANSFER_H

#include <QObject>
#include <QIODevice>
#include "qusbdevice.h"

class QUsbTransferPrivate;

class Q_USB_EXPORT QUsbTransfer : public QIODevice
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QUsbTransfer)

public:
    enum Type {
        controlTransfer = 0,
        isochronousTransfer,
        bulkTransfer,
        interruptTransfer,
        streamTransfer
    };
    Q_ENUM(Type)

    enum Status {
        transferCompleted,
        transferError,
        transferTimeout,
        transferCanceled,
        transferStall,
        transferNoDevice,
        transferOverflow,
    };
    Q_ENUM(Status)

    Q_PROPERTY(Type type READ type) /*!< Transfer type */
    Q_PROPERTY(QUsbDevice::Endpoint endpointIn READ endpointIn) /*!< Input endpoint */
    Q_PROPERTY(QUsbDevice::Endpoint endpointOut READ endpointOut) /*!< Output endpoint */
    Q_PROPERTY(bool polling READ polling WRITE setPolling) /*!< Input Polling */

    explicit QUsbTransfer(QUsbDevice *dev,
                          Type type,
                          QUsbDevice::Endpoint in,
                          QUsbDevice::Endpoint out);
    ~QUsbTransfer();

    bool open(QIODevice::OpenMode mode);
    void close();
    Type type() const;

    QUsbDevice::Endpoint endpointIn(void) const;
    QUsbDevice::Endpoint endpointOut(void) const;

    bool isSequential() const;
    Status status() const;

    qint64 bytesAvailable() const;
    qint64 bytesToWrite() const;
    bool waitForBytesWritten(int msecs);
    bool waitForReadyRead(int msecs);

    void makeControlPacket(char *buffer,
                           quint8 bmRequestType,
                           quint8 bRequest,
                           quint16 wValue,
                           quint16 wIndex,
                           quint16 wLength) const;

    void setPolling(bool enable);
    bool polling();
    bool poll();

public slots:
    void cancelTransfer();

Q_SIGNALS:
    void error(Status);

protected:
    qint64 readData(char *data, qint64 maxSize);
    qint64 writeData(const char *data, qint64 maxSize);

private:
    QUsbTransferPrivate *const d_dummy;
    Q_DISABLE_COPY(QUsbTransfer)

    Status m_status;
    const QUsbDevice *m_dev;
    const Type m_type;
    const QUsbDevice::Endpoint m_in_ep;
    const QUsbDevice::Endpoint m_out_ep;
};

#endif // QUSBTRANSFER_H
