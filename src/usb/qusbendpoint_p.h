#ifndef QUSBENDPOINT_P_H
#define QUSBENDPOINT_P_H

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

#include "qusbendpoint.h"
#include <QMutexLocker>
#include <private/qiodevice_p.h>

#if defined(Q_OS_MACOS)
  #include <libusb.h>
#elif defined(Q_OS_UNIX)
  #include <libusb-1.0/libusb.h>
#else
  #include <libusb/libusb.h>
#endif

QT_BEGIN_NAMESPACE

class QUsbEndpointPrivate : public QIODevicePrivate
{
    Q_DECLARE_PUBLIC(QUsbEndpoint)

public:
    QUsbEndpointPrivate();

    void readyRead();
    void bytesWritten(qint64 bytes);
    void error(QUsbEndpoint::Status error);
    void setStatus(QUsbEndpoint::Status status);
    bool isValid();

    bool prepareTransfer(libusb_transfer **tr, libusb_transfer_cb_fn cb,
                         char *data, qint64 size, quint8 ep);
    void stopTransfer();

    int readUsb(qint64 maxSize);
    int writeUsb(const char *data, qint64 maxSize);

    void setPolling(bool enable);
    bool polling() { return m_poll; }

    QUsb::LogLevel logLevel();

    bool m_poll;
    int m_poll_size;

    libusb_transfer *m_transfer;
    QByteArray m_buf, m_transfer_buf;
    QMutex m_transfer_mutex, m_buf_mutex;
};

QT_END_NAMESPACE

#endif
