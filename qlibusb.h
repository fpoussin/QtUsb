#ifndef QLIBUSB_H
#define QLIBUSB_H

#include <QString>
#include <QDebug>
#include <QByteArray>
#include <QtEndian>

#include <qbaseusb.h>
#include <compat.h>
#include <libusb-1.0/libusb.h>

class QUsb : public QBaseUsb
{
    Q_OBJECT

public:
    explicit QUsb(QBaseUsb *parent = 0);
    ~QUsb();

public slots:
    qint32 open();
    void close();
    qint32 read(QByteArray *buf, quint32 bytes);
    qint32 write(QByteArray *buf, quint32 bytes);
    void setDebug(bool enable);

private:
    libusb_device **mDevs;
    libusb_device_handle *mDevHandle;
    libusb_context *mCtx;

    quint8 mInterface;
    quint8 mAlternate;
};
#endif // QLIBUSB_H
