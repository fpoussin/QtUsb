#include "usbexample.h"

#ifdef interface
#undef interface
#endif

UsbExample::UsbExample(QObject *parent) : QObject(parent) {
  this->setupDevice();

  QByteArray send, recv;

  send.append(static_cast<char>(0xAB));

  if (this->openDevice()) {
    qInfo("Device open!");
    this->write(&send);
    this->read(&recv);
  }
  else {
    qWarning("Could not open device!");
  }
}

UsbExample::~UsbExample() { delete m_usb_dev; }

void UsbExample::setupDevice() {
  /* There are 2 ways of identifying devices depending on the platform.
   * You can use both methods, only one will be taken into account.
   */

  qDebug("setupDevice");

  m_usb_dev = new QUsbDevice();
  m_usb_dev->setDebug(true);

  //
  m_filter.pid = 0x3748;
  m_filter.vid = 0x0483;

  //
  m_config.alternate = 0;
  m_config.config = 0;
  m_config.interface = 1;

  //
  m_read_ep = 0x81;
  m_write_ep = 0x02;

  //
  m_usb_dev->setFilter(m_filter);
  m_usb_dev->setConfig(m_config);
}

bool UsbExample::openDevice() {
  qDebug("Opening");

  if (m_usb_dev->open() == QtUsb::deviceOK) {
    // Device is open
    return true;
  }
  return false;
}

bool UsbExample::closeDevice() {
  qDebug("Closing");
  m_usb_dev->close();
  return false;
}

void UsbExample::read(QByteArray *buf) {
  m_usb_dev->read(buf, 1, m_read_ep);
}

void UsbExample::write(QByteArray *buf) {
  m_usb_dev->write(buf, static_cast<quint32>(buf->size()), m_write_ep);
}
