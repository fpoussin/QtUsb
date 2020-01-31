#include "hidexample.h"

HidExample::HidExample(QObject *parent)
    : QObject(parent), m_hid_dev(new QHidDevice())
{
    m_send.append(static_cast<char>(0xF1u));
    m_send.append(static_cast<char>(0x80u));

    if (this->openDevice()) {
        qInfo("Device open!");
        this->write(&m_send);
    } else {
        qWarning("Could not open device!");
    }
}

HidExample::~HidExample()
{
    this->closeDevice();
    m_hid_dev->deleteLater();
}

bool HidExample::openDevice()
{
    qDebug("Opening");

    return m_hid_dev->open(0x0483, 0x3748);
}

void HidExample::closeDevice()
{
    qDebug("Closing");

    if (m_hid_dev->isOpen()) {
        m_hid_dev->close();
    }
}

void HidExample::read(QByteArray *buf)
{
    QByteArray b(128, '0');
    m_hid_dev->read(&b, b.size());

    qDebug() << "Reading" << b << b.size();
    buf->append(b);
}

void HidExample::write(QByteArray *buf)
{
    qDebug() << "Writing" << *buf << buf->size();
    if (m_hid_dev->write(buf, buf->size()) < 0) {
        qWarning("write failed");
    }
}
