#ifndef QBASEUSB_H
#define QBASEUSB_H

#include <QObject>
#include <QString>
#include <QDebug>
#include "qusb_global.h"

namespace QUSB {

    const quint16 DEFAULT_TIMEOUT_MSEC = 250;

    enum SPEED {unknownSpeed = -1, lowSpeed = 0, fullSpeed, highSpeed, superSpeed};

}

class QUSBSHARED_EXPORT QBaseUsb : public QObject
{
    Q_OBJECT
    
public:
    QBaseUsb(QObject *parent = 0);
    virtual ~QBaseUsb();
    quint16 getTimeout(void);
    void setDebug(bool enable);

    quint16 getPid(void) { return mPid; }
    quint16 getVid(void) { return mVid; }
    QString getGuid(void) { return mGuid; }
    quint8 getReadEp(void) { return mReadEp; }
    quint8 getWriteEp(void) { return mWriteEp; }
    QUSB::SPEED getSpeed(void) { return mSpd; }

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
    quint16 mTimeout;
    quint16 mPid;
    quint16 mVid;
    QString mGuid;
    bool mDebug;
    bool mConnected;
    quint8 mReadEp;
    quint8 mWriteEp;
    int mConfig;
    int mInterface;
    int mAlternate;
    QUSB::SPEED mSpd;
};

#endif // QBASEUSB_H
