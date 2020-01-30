#ifndef QUSBDEVICE_P_H
#define QUSBDEVICE_P_H

#include "qusbdevice.h"
#include <private/qobject_p.h>
#include <QThread>

#ifdef Q_OS_UNIX
#include <libusb-1.0/libusb.h>
#else
#include <libusb/libusb.h>
#endif

QT_BEGIN_NAMESPACE

class QUsbEventsThread : public QThread
{
public:
    void run();

    libusb_context *m_ctx;
};

class QUsbTransferPrivate;

typedef struct {
    QUsbDevicePrivate *priv;
    QUsbDevice *pub;
} qusbdevice_classes_t;

class QUsbDevicePrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QUsbDevice)
    friend QUsbTransferPrivate;

public:
    QUsbDevicePrivate();
    ~QUsbDevicePrivate();

    libusb_device **m_devs;
    libusb_device_handle *m_devHandle;
    libusb_context *m_ctx;
    qusbdevice_classes_t m_classes;

    QUsbEventsThread *m_events;
};

QT_END_NAMESPACE

#endif
