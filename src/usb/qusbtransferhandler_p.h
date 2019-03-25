#ifndef QUSBTRANSFERHANDLER_P_H
#define QUSBTRANSFERHANDLER_P_H

#include "qusbtransferhandler.h"
#include <private/qiodevice_p.h>

#ifdef Q_OS_UNIX
#include <libusb-1.0/libusb.h>
#else
#include <libusb/libusb.h>
#endif

QT_BEGIN_NAMESPACE

class QUsbTransferHandlerPrivate : public QIODevicePrivate {

  Q_DECLARE_PUBLIC(QUsbTransferHandler)

public:
  QUsbTransferHandlerPrivate();

  void readyRead();
  void bytesWritten(qint64 bytes);

  void prepareTransfer(libusb_transfer* tr, libusb_transfer_cb_fn cb, char *data, qint64 size, QtUsb::Endpoint ep);

  libusb_transfer * m_transfer_in;
  libusb_transfer * m_transfer_out;

  QByteArray m_write_buf;
};

QT_END_NAMESPACE

#endif
