#include "usbexample.h"

#ifdef interface
#undef interface
#endif

UsbExample::UsbExample(QObject *parent) : QObject(parent), m_usb_dev(new QUsbDevice()), m_transfer_handler(Q_NULLPTR) {
  this->setupDevice();

  m_send.append(static_cast<char>(0xAB));

  if (this->openDevice()) {
    qInfo("Device open!");
    this->write(&m_send);
  }
  else {
    qWarning("Could not open device!");
  }
}

UsbExample::~UsbExample() {
  this->closeDevice();
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
  m_config.config = 1;
  m_config.interface = 0;

  //
  m_read_ep = 0x81;
  m_write_ep = 0x02;

  //
  m_usb_dev->setFilter(m_filter);
  m_usb_dev->setConfig(m_config);
}

bool UsbExample::openDevice() {
  qDebug("Opening");

  if (m_usb_dev->open() == QUsbDevice::statusOK) {
    // Device is open
    return this->openHandle();
  }
  return false;
}

void UsbExample::closeDevice() {
  qDebug("Closing");

  if (m_usb_dev->isConnected()) {
    this->closeHandle();
    m_usb_dev->close();
  }
}

bool UsbExample::openHandle()
{
  qDebug("Opening Handle");
  bool b = false;
  m_transfer_handler = new QUsbTransferHandler(m_usb_dev, QUsbTransferHandler::bulkTransfer, m_read_ep, m_write_ep);

  connect(m_transfer_handler, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
  connect(m_transfer_handler, SIGNAL(bytesWritten(qint64)), this, SLOT(onWriteComplete(qint64)));

  b = m_transfer_handler->open(QIODevice::ReadWrite);
  if (b) {
    m_transfer_handler->setPolling(true);
  }

  return b;
}

void UsbExample::closeHandle()
{
  qDebug("Closing Handle");
  if (m_transfer_handler != Q_NULLPTR) {
    m_transfer_handler->close();
    m_transfer_handler->disconnect();
    qInfo() << m_transfer_handler->errorString();
    delete m_transfer_handler;
    m_transfer_handler = Q_NULLPTR;
  }
}

void UsbExample::read(QByteArray *buf) {
  *buf = m_transfer_handler->readAll();
}

void UsbExample::write(QByteArray *buf) {
  m_transfer_handler->write(buf->constData(), buf->size());
}

void UsbExample::onReadyRead()
{
  qDebug("Data received");
  this->read(&m_recv);
  qDebug() << m_recv;
}

void UsbExample::onWriteComplete(qint64 bytes)
{
  qDebug("Data written");
  qDebug() << bytes;
}

