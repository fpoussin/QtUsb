#ifndef QUSBDEVICE_P_H
#define QUSBDEVICE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qusbdevice.h"
#include <private/qobject_p.h>
#include <QThread>

#if defined(Q_OS_MACOS)
  #include <libusb.h>
#elif defined(Q_OS_UNIX)
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
    void registerDisconnectCallback(int vid, int pid);
    void deregisterDisconnectCallback();
    ~QUsbDevicePrivate();

    libusb_device **m_devs;
    libusb_device_handle *m_devHandle;
    libusb_context *m_ctx;
    libusb_hotplug_callback_handle m_callbackHandle;
    qusbdevice_classes_t m_classes;

    bool m_hasHotplug;

    QUsbEventsThread *m_events;
};

QT_END_NAMESPACE

#endif
