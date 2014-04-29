#ifndef QBASEUSB_H
#define QBASEUSB_H

#include <QObject>
#include <QString>
#include <QDebug>
#include "qusb_global.h"

const quint16 DEFAULT_TIMEOUT_MSEC = 250;

class QUSBSHARED_EXPORT QBaseUsb : public QObject
{
    Q_OBJECT
    
public:
    QBaseUsb(QObject *parent = 0);
    virtual ~QBaseUsb();
    quint16 getTimeout(void);
    void setDebug(bool enable);

public slots:
    virtual qint32 open() = 0;
    virtual void close() = 0;
    virtual qint32 read(QByteArray *buf, quint32 bytes) = 0;
    virtual qint32 write(QByteArray *buf, quint32 bytes) = 0;

    void setTimeout(quint16 timeout);
    void setEndPoints(quint8 in, quint8 out);
    void setDeviceIds(quint16 pid, quint16 vid);
    bool setGuid(const QString &guid);
    void setConfiguration(quint8 config);
    void setInterface(quint8 interface);

    void showSettings(void);

protected slots:
    void setDefaults(void);

protected:
    ulong mTimeout;
    quint16 mPid;
    quint16 mVid;
    QString mGuid;
    bool mDebug;
    quint8 mReadEp;
    quint8 mWriteEp;
    int mConfig;
    int mInterface;
};

#endif // QBASEUSB_H
