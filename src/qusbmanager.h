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

signals:
    void deviceInserted(QtUsb::UsbFilterList filters);
    void deviceRemoved(QtUsb::UsbFilterList filters);

public slots:
    bool addDevice(const QtUsb::UsbDeviceFilter &filter);
    bool removeDevice(const QtUsb::UsbDeviceFilter &filter);
    int findDevice(const QtUsb::UsbDeviceFilter &filter, QtUsb::UsbFilterList &list);

    QtUsb::DeviceStatus openDevice(QUsbDevice* dev, const QtUsb::UsbDeviceFilter &filter, const QtUsb::UsbDeviceConfig &config);
    QtUsb::DeviceStatus closeDevice(QUsbDevice* dev);


protected slots:
    void monitorDevices(QtUsb::UsbFilterList& list);
    void run(void);

protected:
    bool mStop;
    QList<QUsbDevice*> mUsedDeviceList;
    QList<QtUsb::UsbDeviceFilter> mFilterList;
    QList<QtUsb::UsbDeviceFilter> mSystemList;

};

#endif // QUSBMANAGER_H
