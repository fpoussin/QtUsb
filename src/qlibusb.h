#ifndef QLIBUSB_H
#define QLIBUSB_H

#include <QString>
#include <QDebug>
#include <QByteArray>
#include <QtEndian>

#include <qbaseusb.h>
#include <compat.h>
#include <libusb-1.0/libusb.h>

class QUSBSHARED_EXPORT QUsbDevice : public QBaseUsbDevice
{
    Q_OBJECT

public:
    explicit QUsbDevice(QBaseUsbDevice *parent = 0);
    static QtUsb::UsbFilterList getAvailableDevices(void);
    ~QUsbDevice();

public slots:
    bool open(OpenMode mode);
    qint32 open();
    void close();
    qint32 read(QByteArray *buf, quint32 bytes);
    qint32 write(QByteArray *buf, quint32 bytes);
    void setDebug(bool enable);

private slots:
    void printUsbError(int error_code);
    qint64 readData(char * data, qint64 maxSize);
    qint64 writeData(const char * data, qint64 maxSize);

private:
    libusb_device **mDevs;
    libusb_device_handle *mDevHandle;
    libusb_context *mCtx;
};
#endif // QLIBUSB_H
