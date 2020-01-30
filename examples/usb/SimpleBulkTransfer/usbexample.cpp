#include "usbexample.h"

#ifdef interface
#undef interface
#endif

UsbExample::UsbExample(QObject *parent)
    : QObject(parent), m_usb_dev(new QUsbDevice()), m_read_ep(Q_NULLPTR), m_write_ep(Q_NULLPTR)
{
    this->setupDevice();

    m_send.append(static_cast<char>(0xF1u));
    m_send.append(static_cast<char>(0x80u));

    if (this->openDevice()) {
        qInfo("Device open!");
        this->write(&m_send);
    } else {
        qWarning("Could not open device!");
    }
}

UsbExample::~UsbExample()
{
    this->closeDevice();
    m_usb_dev->deleteLater();
}

void UsbExample::setupDevice()
{
    /* There are 2 ways of identifying devices depending on the platform.
   * You can use both methods, only one will be taken into account.
   */

    qDebug("setupDevice");

    m_usb_dev->setLogLevel(QUsbDevice::logDebug);

    //
    m_filter.pid = 0x3748;
    m_filter.vid = 0x0483;

    //
    m_config.alternate = 0;
    m_config.config = 1;
    m_config.interface = 0;

    //
    m_usb_dev->setId(m_filter);
    m_usb_dev->setConfig(m_config);
}

bool UsbExample::openDevice()
{
    qDebug("Opening");

    if (m_usb_dev->open() == QUsbDevice::statusOK) {
        // Device is open
        return this->openHandle();
    }
    return false;
}

void UsbExample::closeDevice()
{
    qDebug("Closing");

    if (m_usb_dev->isConnected()) {
        this->closeHandle();
        m_usb_dev->close();
    }
}

bool UsbExample::openHandle()
{
    qDebug("Opening Handle");
    bool a = false, b = false;
    m_read_ep = new QUsbEndpoint(m_usb_dev, QUsbEndpoint::bulkEndpoint, USB_ENDPOINT_IN);
    m_write_ep = new QUsbEndpoint(m_usb_dev, QUsbEndpoint::bulkEndpoint, USB_ENDPOINT_OUT);

    connect(m_read_ep, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(m_write_ep, SIGNAL(bytesWritten(qint64)), this, SLOT(onWriteComplete(qint64)));

    a = m_read_ep->open(QIODevice::ReadOnly);
    if (a) {
        m_read_ep->setPolling(true);
    }

    b = m_write_ep->open(QIODevice::WriteOnly);

    return a && b;
}

void UsbExample::closeHandle()
{
    qDebug("Closing Handle");
    if (m_read_ep != Q_NULLPTR) {
        m_read_ep->close();
        m_read_ep->disconnect();
        qInfo() << m_read_ep->errorString();
        delete m_read_ep;
        m_read_ep = Q_NULLPTR;
    }

    if (m_write_ep != Q_NULLPTR) {
        m_write_ep->close();
        m_write_ep->disconnect();
        qInfo() << m_write_ep->errorString();
        delete m_write_ep;
        m_write_ep = Q_NULLPTR;
    }
}

void UsbExample::read(QByteArray *buf)
{
    QByteArray b(m_read_ep->readAll());

    qDebug() << "Reading" << b << b.size();
    buf->append(b);
}

void UsbExample::write(QByteArray *buf)
{
    qDebug() << "Writing" << *buf << buf->size();
    if (m_write_ep->write(buf->constData(), buf->size()) < 0) {
        qWarning("write failed");
    }
}

void UsbExample::onReadyRead()
{
    qDebug("onReadyRead");
    this->read(&m_recv);
    this->write(&m_send);
}

void UsbExample::onWriteComplete(qint64 bytes)
{
    qDebug() << "onWriteComplete" << bytes << "bytes";
}
