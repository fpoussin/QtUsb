#include "qhiddevice.h"
#include "qhiddevice_p.h"

QHidDevice::QHidDevice(QObject *parent)
    : QObject(*(new QHidDevicePrivate), parent), d_dummy(Q_NULLPTR)
{
}

QHidDevice::~QHidDevice()
{
}

bool QHidDevice::open(quint16 vid, quint16 pid, const QString &serial)
{
    Q_D(QHidDevice);
    wchar_t *s = Q_NULLPTR;
    if (!serial.isEmpty()) {
        s = new wchar_t[50];
        serial.left(50).toWCharArray(s);
    }
    d->m_devHandle = hid_open(vid, pid, s);
    delete[] s;
    return d->m_devHandle != Q_NULLPTR;
}

void QHidDevice::close()
{
    Q_D(QHidDevice);
    hid_close(d->m_devHandle);
    d->m_devHandle = Q_NULLPTR;
}

bool QHidDevice::isOpen() const
{
    return d_func()->m_devHandle != Q_NULLPTR;
}

qint32 QHidDevice::write(const QByteArray *data, qint32 len)
{
    Q_CHECK_PTR(data);
    Q_D(QHidDevice);
    // Default is buffer size
    if (len == -1)
        len = data->size();
    return hid_write(d->m_devHandle, (const unsigned char *)data->constData(), len);
}

qint32 QHidDevice::read(QByteArray *data, qint32 len)
{
    Q_CHECK_PTR(data);
    Q_D(QHidDevice);
    // Default is buffer size
    if (len == -1)
        len = data->size();
    // Allocate max read size
    data->fill(0, len);
    int res = hid_read(d->m_devHandle, (unsigned char *)data->data(), len);
    // Resize to actual read size
    data->resize(res);
    return res;
}

QString QHidDevice::serialNumber()
{
    if (!isOpen())
        return QString::fromLocal8Bit("Device Closed");
    Q_D(QHidDevice);
    Q_CHECK_PTR(d->m_devHandle);
    wchar_t *buf = new wchar_t[50];

    hid_get_serial_number_string(d->m_devHandle, buf, 50);
    QString out = QString::fromWCharArray(buf);
    delete[] buf;

    return out;
}

QString QHidDevice::manufacturer()
{
    if (!isOpen())
        return QString::fromLocal8Bit("Device Closed");
    Q_D(QHidDevice);
    Q_CHECK_PTR(d->m_devHandle);
    wchar_t *buf = new wchar_t[50];

    hid_get_manufacturer_string(d->m_devHandle, buf, 50);
    QString out = QString::fromWCharArray(buf);
    delete[] buf;

    return out;
}

QString QHidDevice::product()
{
    if (!isOpen())
        return QString::fromLocal8Bit("Device Closed");
    Q_D(QHidDevice);
    Q_CHECK_PTR(d->m_devHandle);
    wchar_t *buf = new wchar_t[50];

    hid_get_product_string(d->m_devHandle, buf, 50);
    QString out = QString::fromWCharArray(buf);
    delete[] buf;

    return out;
}

QHidDevicePrivate::QHidDevicePrivate()
    : m_devHandle(Q_NULLPTR)
{
    hid_init();
}

QHidDevicePrivate::~QHidDevicePrivate()
{
}
