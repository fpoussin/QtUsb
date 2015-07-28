#ifndef USBEXAMPLE_H
#define USBEXAMPLE_H

#include <QObject>
#include "QUsb"

const quint8 USB_PIPE_IN = 0x81;   /* Bulk output endpoint for responses */
const quint8 USB_PIPE_OUT = 0x01;	   /* Bulk input endpoint for commands */
const quint16 USB_TIMEOUT_MSEC = 300;

class UsbExample : public QObject
{
    Q_OBJECT
public:
    explicit UsbExample(QObject *parent = 0);
    ~UsbExample(void);
    void setupDevice(void);
    bool openDevice(void);
    bool closeDevice(void);
    void read(QByteArray *buf);
    void write(QByteArray *buf);

signals:

public slots:

private:
    QUsb mUsb;

};

#endif // USBEXAMPLE_H
