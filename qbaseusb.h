#ifndef QBASEUSB_H
#define QBASEUSB_H

#include <QObject>
#include <QString>
#include <QDebug>
#include <QVector>
#include <QThread>
#include "qusb_global.h"

namespace QUSB {

    const quint16 DEFAULT_TIMEOUT_MSEC = 250;

    enum Speed {unknownSpeed = -1, lowSpeed = 0, fullSpeed, highSpeed, superSpeed};

    typedef struct {
        quint16 pid;
        quint16 vid;
        QString guid;
        quint8 readEp;
        quint8 writeEp;
        quint8 config;
        quint8 interface;
        quint8 alternate;
    } device;
}

class QUSBSHARED_EXPORT QBaseUsb : public QThread
{
    Q_OBJECT
    
public:
    QBaseUsb(QObject *parent = 0);
    virtual ~QBaseUsb();
    quint16 getTimeout(void);
    void setDebug(bool enable);

    quint16 getPid(void) { return mDevice.pid; }
    quint16 getVid(void) { return mDevice.vid; }
    QString getGuid(void) { return mDevice.guid; }
    quint8 getReadEp(void) { return mDevice.readEp; }
    quint8 getWriteEp(void) { return mDevice.writeEp; }
    QUSB::Speed getSpeed(void) { return mSpd; }
    QUSB::device getDevice(void) { return mDevice; }

signals:
    void deviceInserted(quint16 pid, quint16 vid);
    void deviceRemoved(quint16 pid, quint16 vid);
    void dataReceived(QByteArray buf, quint32 bytes);
    void dataSent(quint32 bytes);

public slots:
    virtual qint32 open() = 0;
    virtual void close() = 0;
    virtual qint32 read(QByteArray *buf, quint32 bytes) = 0;
    virtual qint32 write(QByteArray *buf, quint32 bytes) = 0;

    bool addDevice(const QUSB::device& dev);
    bool removeDevice(const QUSB::device& dev);
    int findDevice(const QUSB::device &filter, QUSB::device* dev = NULL);

    void setTimeout(quint16 timeout);
    void setEndPoints(quint8 in, quint8 out);
    void setDeviceIds(quint16 pid, quint16 vid);
    bool setGuid(const QString &guid);
    void setConfiguration(quint8 config);
    void setInterface(quint8 interface);

    void showSettings(void);

protected slots:
    void setDefaults(void);
    void monitorDevices(void);
    void run(void);

protected:
    QVector<QUSB::device> mDeviceList;
    QUSB::device mDevice;
    bool mStop;
    quint16 mTimeout;
    QString mGuid;
    bool mDebug;
    bool mConnected;
    int mConfig;
    QUSB::Speed mSpd;
};

#endif // QBASEUSB_H
