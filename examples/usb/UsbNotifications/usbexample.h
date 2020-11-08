#ifndef USBEXAMPLE_H
#define USBEXAMPLE_H

#include <QObject>
#include <QUsbDevice>
#include <QUsb>

class UsbExample : public QObject
{
    Q_OBJECT
public:
    explicit UsbExample(QObject *parent = Q_NULLPTR);
    ~UsbExample(void);

signals:

public slots:
    void onDevInserted(QUsb::Id id);
    void onDevRemoved(QUsb::Id id);

private:
    QUsb m_usb_info;
};

#endif // USBEXAMPLE_H
