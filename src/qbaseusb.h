#ifndef QBASEUSB_H
#define QBASEUSB_H

#include <QString>
#include <QDebug>
#include <QList>
#include "qusb_global.h"
#include "qusb_types.h"

class QUSBSHARED_EXPORT QBaseUsbDevice : public QObject
{
    Q_OBJECT
    
public:
    QBaseUsbDevice(QObject *parent = 0);
    virtual ~QBaseUsbDevice();

    void setDebug(bool enable) { mDebug = enable; }
    void setFilter(const QtUsb::DeviceFilter& filter) { mFilter = filter; }
    void setConfig(const QtUsb::DeviceConfig& config) { mConfig = config; }

    QtUsb::DeviceFilter getFilter(void) { return mFilter; }
    QtUsb::DeviceConfig getConfig(void) { return mConfig; }

    quint16 getPid(void) { return mFilter.pid; }
    quint16 getVid(void) { return mFilter.vid; }
    QString getGuid(void) { return mFilter.guid; }
    quint8 getReadEp(void) { return mConfig.readEp; }
    quint8 getWriteEp(void) { return mConfig.writeEp; }
    quint16 getTimeout(void) { return mTimeout; }

    QtUsb::DeviceSpeed getSpeed(void) { return mSpd; }

    static QtUsb::FilterList getAvailableDevices(void);

public slots:
    virtual qint32 open() = 0;
    virtual void close() = 0;
    virtual void flush() = 0;

    virtual qint32 write(const QByteArray* buf, quint32 maxSize) = 0;
    virtual qint32 read(QByteArray* buf, quint32 maxSize) = 0;

    qint32 write(const QByteArray& buf);
    qint32 read(QByteArray* buf);
    bool write(char c);
    bool read(char* c);

    void setTimeout(quint16 timeout);
    void showSettings(void);

protected slots:
    void setDefaults(void);

protected:
    QtUsb::DeviceFilter mFilter;
    QtUsb::DeviceConfig mConfig;
    quint16 mTimeout;
    bool mDebug;
    bool mConnected;
    QtUsb::DeviceSpeed mSpd;
};

#endif // QBASEUSB_H
