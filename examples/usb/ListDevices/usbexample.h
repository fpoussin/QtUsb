#ifndef USBEXAMPLE_H
#define USBEXAMPLE_H

#include <QtCore/QObject>
#include <QtUsb/QUsbDevice>
#include <QtUsb/QUsb>

class UsbExample : public QObject
{
    Q_OBJECT
public:
    explicit UsbExample(QObject *parent = Q_NULLPTR);

private:
    QUsb m_usb;
};

#endif // USBEXAMPLE_H
