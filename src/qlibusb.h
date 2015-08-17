#ifndef QLIBUSB_H
#define QLIBUSB_H

#include <QString>
#include <QDebug>
#include <QByteArray>
#include <QtEndian>
#include <libusb-1.0/libusb.h>
#include "qbaseusb.h"
#include "qusb_compat.h"

class QUSBSHARED_EXPORT QUsbDevice : public QBaseUsbDevice
{
    Q_OBJECT

public:
    explicit QUsbDevice(QBaseUsbDevice *parent = 0);
    ~QUsbDevice();

    static QtUsb::FilterList getAvailableDevices(void);

public slots:
    qint32 open();
    void close();

    void flush();
    qint32 read(QByteArray* buf, quint32 maxSize);
    qint32 write(const QByteArray* buf, quint32 maxSize);

    void setDebug(bool enable);

private slots:    

private:
    void printUsbError(int error_code);
    libusb_device **mDevs;
    libusb_device_handle *mDevHandle;
    libusb_context *mCtx;
};
#endif // QLIBUSB_H
