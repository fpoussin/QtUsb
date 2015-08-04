#ifndef QUSBMANAGER_H
#define QUSBMANAGER_H

#include <QThread>
#include <QList>
#include "qusb_global.h"
#include "qusbtypes.h"
#include "qusb.h"

class QUSBSHARED_EXPORT QUsbManager : public QThread
{
    Q_OBJECT
public:
    explicit QUsbManager(QObject *parent = 0);
    ~QUsbManager(void);

    QtUsb::FilterList getPresentDevices(void);
    bool isPresent(const QtUsb::DeviceFilter &filter);

signals:
    void deviceInserted(QtUsb::FilterList filters);
    void deviceRemoved(QtUsb::FilterList filters);

public slots:
    bool addDevice(const QtUsb::DeviceFilter &filter);
    bool removeDevice(const QtUsb::DeviceFilter &filter);
    int findDevice(const QtUsb::DeviceFilter &filter, const QtUsb::FilterList &list);

    QtUsb::DeviceStatus openDevice(QUsbDevice* dev, const QtUsb::DeviceFilter &filter, const QtUsb::DeviceConfig &config);
    QtUsb::DeviceStatus closeDevice(QUsbDevice* dev);

protected slots:
    void monitorDevices(const QtUsb::FilterList &list);
    void run(void);

protected:
    bool mStop;
    QList<QUsbDevice*> mUsedDeviceList;
    QList<QtUsb::DeviceFilter> mFilterList;
    QList<QtUsb::DeviceFilter> mSystemList;

};

#endif // QUSBMANAGER_H
