#include "usbexample.h"

#ifdef interface
#undef interface
#endif

UsbExample::UsbExample(QObject *parent) : QObject(parent), m_usb_dev(new QUsbDevice()) {
  this->setupDevice();

  m_send.append(static_cast<char>(0xAB));

  if (this->openDevice()) {
    qInfo("Device open!");
    this->write(&m_send);
    this->read(&m_recv);
  }
  else {
    qWarning("Could not open device!");
  }

  this->closeDevice();
}

UsbExample::~UsbExample() {
  m_usb_dev->deleteLater();
}

void UsbExample::setupDevice() {
  /* There are 2 ways of identifying devices depending on the platform.
   * You can use both methods, only one will be taken into account.
   */

  qDebug("setupDevice");

  m_usb_dev->setDebug(false);

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

  if (m_usb_dev->open() == QtUsb::statusOK) {
    // Device is open
    m_transfer_handler = new QUsbTransferHandler(m_usb_dev, QtUsb::bulkTransfer, m_read_ep, m_write_ep);

    connect(m_transfer_handler, SIGNAL(readyRead()), this, SLOT(onReadComplete()));
    connect(m_transfer_handler, SIGNAL(bytesWritten(qint64)), this, SLOT(onWriteComplete(qint64)));
    return true;
  }
  return false;
}

bool UsbExample::closeDevice() {
  qDebug("Closing");

  m_transfer_handler->disconnect();
  delete m_transfer_handler;
  m_usb_dev->close();
  return false;
}

void UsbExample::read(QByteArray *buf) {
  buf->resize(1);
  m_transfer_handler->read(buf->data(), 1);
}

void UsbExample::write(QByteArray *buf) {
  m_transfer_handler->write(buf->constData(), buf->size());
}

void UsbExample::onReadComplete()
{
  qDebug("Data received");
  qDebug() << m_recv;
}

void UsbExample::onWriteComplete(qint64 bytes)
{
  qDebug("Data written");
  qDebug() << bytes;
}

