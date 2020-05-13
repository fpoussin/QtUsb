#include "qhiddevice.h"
#include "qhiddevice_p.h"

/*!
    \class QHidDevice

    \brief This class handles all HID operations

    \reentrant
    \ingroup usb-main
    \inmodule QtUsb
*/

QHidDevice::QHidDevice(QObject *parent)
    : QObject(*(new QHidDevicePrivate), parent), d_dummy(Q_NULLPTR)
{
}

QHidDevice::~QHidDevice()
{
}

/*!
    \brief Opens the HID device, using \a vid Vendor ID, \a pid Product ID, and an optional \a serial number. Returns \c true on sucess.
 */
bool QHidDevice::open(quint16 vid, quint16 pid, const QString *serial)
{
    Q_D(QHidDevice);
    wchar_t *s = Q_NULLPTR;
    if (serial != Q_NULLPTR && !serial->isEmpty()) {
        s = new wchar_t[serial->size()];
        serial->toWCharArray(s);
    }
    d->m_devHandle = hid_open(vid, pid, s);
    delete[] s;
    return d->m_devHandle != Q_NULLPTR;
}

/*!
    \brief Close the device.
 */
void QHidDevice::close()
{
    Q_D(QHidDevice);
    hid_close(d->m_devHandle);
    d->m_devHandle = Q_NULLPTR;
}

/*!
    \brief Returns \c true if device is open.
 */
bool QHidDevice::isOpen() const
{
    return d_func()->m_devHandle != Q_NULLPTR;
}

/*!
    \brief Write \a data to device.

    \a len defaults to the size of \a data.
 */
qint32 QHidDevice::write(const QByteArray *data, int len)
{
    Q_CHECK_PTR(data);
    Q_D(QHidDevice);
    // Default is buffer size
    if (len == -1)
        len = data->size();
    return hid_write(d->m_devHandle, reinterpret_cast<const unsigned char *>(data->constData()), static_cast<size_t>(len));
}

/*!
    \brief Read from device to \a data.

    \a len defaults to the size of \a data.
    \a timeout defaults to -1 (unlimited, blocking).
 */
qint32 QHidDevice::read(QByteArray *data, int len, int timeout)
{
    Q_CHECK_PTR(data);
    Q_D(QHidDevice);
    // Default is buffer size
    if (len == -1)
        len = data->size();
    // Allocate max read size
    data->fill(0, len);
    int res = hid_read_timeout(d->m_devHandle, reinterpret_cast<unsigned char *>(data->data()), static_cast<size_t>(len), timeout);
    // Resize to actual read size
    data->resize(res);
    return res;
}

/*!
    \brief Send a Feature report.
 */
qint32 QHidDevice::sendFeatureReport(const QByteArray *data, int len)
{
    Q_CHECK_PTR(data);
    Q_D(QHidDevice);
    // Default is buffer size
    if (len == -1)
        len = data->size();

    return hid_send_feature_report(d->m_devHandle, reinterpret_cast<const unsigned char *>(data->constData()), static_cast<size_t>(len));
}

/*!
    \brief Get a feature report.
 */
qint32 QHidDevice::getFeatureReport(QByteArray *data, int len)
{
    Q_CHECK_PTR(data);
    Q_D(QHidDevice);
    // Default is buffer size
    if (len == -1)
        len = data->size();
    // Allocate max read size
    data->fill(0, len);

    return hid_get_feature_report(d->m_devHandle, reinterpret_cast<unsigned char *>(data->data()), static_cast<size_t>(len));
}

/*!
    \brief Returns the serial number string.
 */
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

/*!
    \brief Returns the manufacturer string.
 */
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

/*!
    \brief Returns the product string.
 */
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
