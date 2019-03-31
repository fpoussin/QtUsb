#ifndef QUSBTRANSFER_P_H
#define QUSBTRANSFER_P_H

#include "qusbtransfer.h"
#include <private/qiodevice_p.h>
#include <QMutexLocker>

#ifdef Q_OS_UNIX
#include <libusb-1.0/libusb.h>
#else
#include <libusb/libusb.h>
#endif

QT_BEGIN_NAMESPACE

class QUsbTransferPrivate : public QIODevicePrivate
{
    Q_DECLARE_PUBLIC(QUsbTransfer)

public:
    QUsbTransferPrivate();

    void readyRead();
    void bytesWritten(qint64 bytes);
    void error(QUsbTransfer::Status error);
    void setStatus(QUsbTransfer::Status status);
    bool isValid();

    bool prepareTransfer(libusb_transfer **tr, libusb_transfer_cb_fn cb, char *data, qint64 size, QUsbDevice::Endpoint ep);
    void stopTransfer();

    int readUsb(qint64 maxSize);
    int writeUsb(const char *data, qint64 maxSize);

    void setPolling(bool enable);
    bool polling() { return m_poll; }

    QUsbDevice::LogLevel logLevel();

    bool m_poll;
    int m_poll_size;

    libusb_transfer *m_transfer_in, *m_transfer_out;
    QByteArray m_write_buf, m_read_buf, m_read_transfer_buf;
    QMutex m_read_transfer_mutex, m_read_buf_mutex, m_write_buf_mutex;
};

QT_END_NAMESPACE

#endif
