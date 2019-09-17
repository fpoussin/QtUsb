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
    enum Type : quint8 {
        controlTransfer = 0,
        isochronousTransfer,
        bulkTransfer,
        interruptTransfer,
        streamTransfer
    };
    Q_ENUM(Type)

    enum Status : quint8 {
        transferCompleted,
        transferError,
        transferTimeout,
        transferCanceled,
        transferStall,
        transferNoDevice,
        transferOverflow,
    };
    Q_ENUM(Status)

    enum bmRequestType : quint8 {
        requestStandard = (0x00 < 5),
        requestClass = (0x01 < 5),
        requestVendor = (0x02 < 5),
        requestReserved = (0x03 < 5),
        recipientDevice = 0x00,
        recipientInterface = 0x01,
        recipientEndpoint = 0x02,
        recipientOther = 0x03
    };
    Q_ENUM(bmRequestType)

    enum bRequest : quint8 {
        requestGetStatus = 0x00,
        requestClearFeature = 0x01,
        requestSetFeature = 0x03,
        requestSetAddress = 0x05,
        requestGetDescriptor = 0x06,
        requestSetDescriptor = 0x07,
        requestGetConfiguration = 0x08,
        requestSetConfiguration = 0x09,
        requestGetInterface = 0x0A,
        requestSetInterface = 0x0B,
        requestSynchFrame = 0x0C,
        requestSetSel = 0x30,
        requestIsochDelay = 0x31
    };
    Q_ENUM(bRequest)

    Q_PROPERTY(Type type READ type)
    Q_PROPERTY(QUsbDevice::Endpoint endpointIn READ endpointIn)
    Q_PROPERTY(QUsbDevice::Endpoint endpointOut READ endpointOut)
    Q_PROPERTY(bool polling READ polling WRITE setPolling)

    explicit QUsbTransfer(QUsbDevice *dev,
                          Type type,
                          QUsbDevice::Endpoint in,
                          QUsbDevice::Endpoint out);
    ~QUsbTransfer();

    bool open(QIODevice::OpenMode mode);
    void close();
    Type type() const;

    QUsbDevice::Endpoint endpointIn() const;
    QUsbDevice::Endpoint endpointOut() const;

    bool isSequential() const;
    Status status() const;

    qint64 bytesAvailable() const;
    qint64 bytesToWrite() const;
    bool waitForBytesWritten(int msecs);
    bool waitForReadyRead(int msecs);

    void makeControlPacket(char *buffer,
                           QUsbTransfer::bmRequestType bmRequestType,
                           QUsbTransfer::bRequest bRequest,
                           quint16 wValue,
                           quint16 wIndex,
                           quint16 wLength) const;

    void setPolling(bool enable);
    bool polling();
    bool poll();

public slots:
    void cancelTransfer();

Q_SIGNALS:
    void error(QUsbTransfer::Status error);

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
