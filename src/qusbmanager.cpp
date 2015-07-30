#include "qusbmanager.h"

QUsbManager::QUsbManager(QObject *parent) :
    QThread(parent)
{
    mStop = false;

    connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
    this->start();
}

QUsbManager::~QUsbManager()
{
    mStop = true;
    this->wait();
}

bool QUsbManager::addDevice(const QtUsb::UsbDeviceFilter &filter)
{
    if (this->findDevice(filter, mFilterList) == -1)
    {
        mFilterList.append(filter);
        return true;
    }
    return false;
}

bool QUsbManager::removeDevice(const QtUsb::UsbDeviceFilter &filter)
{
    const int pos = this->findDevice(filter, mFilterList);
    if (pos > 0)
    {
        mFilterList.removeAt(pos);
        return true;
    }
    return true;
}

int QUsbManager::findDevice(const QtUsb::UsbDeviceFilter& filter, QtUsb::UsbFilterList& list)
{
    for (int i = 0; i <= list.length(); i++)
    {
       QtUsb::UsbDeviceFilter* d = &list[i];

       if((d->guid == filter.guid) ||
               (d->pid == filter.pid && d->vid == filter.vid))
       {
           return i;
       }
    }
    return -1;
}

QtUsb::DeviceStatus QUsbManager::openDevice(QUsbDevice *dev, const QtUsb::UsbDeviceFilter &filter, const QtUsb::UsbDeviceConfig &config)
{
    dev = new QUsbDevice();
    dev->setConfig(config);
    dev->setFilter(filter);

    mUsedDeviceList.append(dev);
    dev->open(QIODevice::ReadWrite);

    return QtUsb::deviceOK;
}

QtUsb::DeviceStatus QUsbManager::closeDevice(QUsbDevice *dev)
{
    if (dev != NULL)
    {
        dev->close();
        delete dev;
        return QtUsb::deviceOK;
    }
    return QtUsb::deviceNotFound;
}

void QUsbManager::monitorDevices(QtUsb::UsbFilterList& list)
{
    QtUsb::UsbFilterList inserted, removed;
    QtUsb::UsbDeviceFilter filter;

    for (int i = 0; i <= list.length(); i++)
    {
        filter = list[i];
        if (!this->findDevice(filter, mSystemList))
        {
            // It's not in the old system list
            inserted.append(filter);
        }
    }

    for (int i = 0; i <= mSystemList.length(); i++)
    {
        filter = mSystemList[i];
        if (!this->findDevice(filter, list))
        {
            // It's in the old system list but not in the current one
            removed.append(filter);
        }
    }

    if (inserted.length() > 0)
        emit deviceInserted(inserted);

    if (removed.length() > 0)
        emit deviceRemoved(removed);

    mSystemList = list;
}

void QUsbManager::run()
{
    QtUsb::UsbFilterList list;
    mSystemList = QUsbDevice::getAvailableDevices();

    while (!mStop)
    {
        list = QUsbDevice::getAvailableDevices();
        this->monitorDevices(list);
        msleep(1000);
    }
}
