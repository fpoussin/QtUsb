#ifndef QUSBDEVICE_P_H
#define QUSBDEVICE_P_H

#include "qusbdevice.h"
#include <private/qobject_p.h>

#ifdef Q_OS_UNIX
#include <libusb-1.0/libusb.h>
#else
#include <libusb/libusb.h>
#endif

QT_BEGIN_NAMESPACE

class QUsbTransferHandlerPrivate;

class QUsbDevicePrivate : public QObjectPrivate {

  Q_DECLARE_PUBLIC(QUsbDevice)
  friend QUsbTransferHandler;
  friend QUsbTransferHandlerPrivate;

public:
  QUsbDevicePrivate();

  /**
   * @brief Print error code to qWarning
   *
   * @param error_code
   */
  void printUsbError(int error_code) const;

  /**
   * @brief Set default values (config)
   *
   */
  void setDefaults(void);

  libusb_device **m_devs;            /**< libusb device ptr to ptr */
  libusb_device_handle *m_devHandle; /**< libusb device handle ptr */
  libusb_context *m_ctx;             /**< libusb context */
};

QT_END_NAMESPACE

#endif
