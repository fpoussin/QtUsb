#ifndef QBASEUSB_H
#define QBASEUSB_H

#include <QString>
#include <QDebug>
#include <QList>
#include <QIODevice>
#include "qusb_global.h"
#include "qusbtypes.h"

class QUSBSHARED_EXPORT QBaseUsbDevice : public QIODevice
{
    Q_OBJECT
    
public:
    QBaseUsbDevice(QObject *parent = 0);
    virtual ~QBaseUsbDevice();
    quint16 getTimeout(void);
    void setDebug(bool enable);

    bool isSequential() const { return true;  }

    void setFilter(const QtUsb::DeviceFilter& filter) { mFilter = filter; }
    void setConfig(const QtUsb::DeviceConfig& config) { mConfig = config; }

    QtUsb::DeviceFilter getFilter(void) { return mFilter; }
    QtUsb::DeviceConfig getConfig(void) { return mConfig; }

    quint16 getPid(void) { return mFilter.pid; }
    quint16 getVid(void) { return mFilter.vid; }
    QString getGuid(void) { return mFilter.guid; }
    quint8 getReadEp(void) { return mConfig.readEp; }
    quint8 getWriteEp(void) { return mConfig.writeEp; }

    QtUsb::DeviceSpeed getSpeed(void) { return mSpd; }

    static QtUsb::FilterList getAvailableDevices(void);

public slots:
    virtual bool open(OpenMode mode) = 0;
    virtual qint32 open() = 0;
    virtual void close();

    void setTimeout(quint16 timeout);
    void showSettings(void);

protected slots:
    void setDefaults(void);

    virtual qint64 readData(char * data, qint64 maxSize) = 0;
    virtual qint64 writeData(const char * data, qint64 maxSize) = 0;

protected:
    QtUsb::DeviceFilter mFilter;
    QtUsb::DeviceConfig mConfig;
    quint16 mTimeout;
    bool mDebug;
    bool mConnected;
    QtUsb::DeviceSpeed mSpd;
};

#endif // QBASEUSB_H
