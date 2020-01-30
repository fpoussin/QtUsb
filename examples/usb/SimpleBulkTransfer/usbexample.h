#ifndef USBEXAMPLE_H
#define USBEXAMPLE_H

#include <QObject>
#include <QUsbDevice>
#include <QUsbEndpoint>

const quint8 USB_ENDPOINT_IN = 0x81; /* Bulk output endpoint for responses */
const quint8 USB_ENDPOINT_OUT = 0x01; /* Bulk input endpoint for commands */
const quint16 USB_TIMEOUT_MSEC = 300;

class UsbExample : public QObject
{
    Q_OBJECT
public:
    explicit UsbExample(QObject *parent = Q_NULLPTR);
    ~UsbExample(void);
    void setupDevice(void);
    bool openDevice(void);
    void closeDevice(void);
    bool openHandle(void);
    void closeHandle(void);
    void read(QByteArray *buf);
    void write(QByteArray *buf);

public slots:
    void onReadyRead(void);
    void onWriteComplete(qint64 bytes);

signals:

private:
    QUsbDevice *m_usb_dev;
    QUsbEndpoint *m_read_ep, *m_write_ep;

    QByteArray m_send, m_recv;

    QUsbDevice::Id m_filter;
    QUsbDevice::Config m_config;

};

#endif // USBEXAMPLE_H
