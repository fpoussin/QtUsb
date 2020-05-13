#ifndef QHIDDEVICE_H
#define QHIDDEVICE_H

#include <QObject>
#include "qusbdevice.h"

QT_BEGIN_NAMESPACE

class QHidDevicePrivate;

class Q_USB_EXPORT QHidDevice : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QHidDevice)

public:
    explicit QHidDevice(QObject *parent = Q_NULLPTR);
    ~QHidDevice();
    bool open(quint16 vid, quint16 pid, const QString *serial = Q_NULLPTR);
    void close();

    bool isOpen() const;

    qint32 write(const QByteArray *data, int len = -1);
    qint32 read(QByteArray *data, int len = -1, int timeout = -1);

    qint32 sendFeatureReport(const QByteArray *data, int len = -1);
    qint32 getFeatureReport(QByteArray *data, int len = -1);

    QString serialNumber();
    QString manufacturer();
    QString product();

private:
    QHidDevicePrivate *const d_dummy;
    Q_DISABLE_COPY(QHidDevice)
};

QT_END_NAMESPACE

#endif // QHIDDEVICE_H
