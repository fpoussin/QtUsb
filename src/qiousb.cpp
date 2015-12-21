#include "qiousb.h"

QUsbDevice::QUsbDevice(QBaseUsbDevice *parent) :
    QBaseUsbDevice(parent)
{
    mUsbInterface = 0;
    mUsbDevice = 0;
    mConfig = 0;
    mInterfaceRequest = 0;
    mUsbInterface = 0;
    mMatchingDictionary = IOServiceMatching(kIOUSBDeviceClassName);
}

QUsbDevice::~QUsbDevice()
{

}

QtUsb::FilterList QUsbDevice::getAvailableDevices()
{
    io_iterator_t iterator = 0;
    io_service_t usbRef;
    CFDictionaryAddValue(mMatchingDictionary,
                         CFSTR(kUSBVendorID),
                         CFNumberCreate(kCFAllocatorDefault,
                                        kCFNumberSInt32Type, &mFilter.vid));
    CFDictionaryAddValue(mMatchingDictionary,
                         CFSTR(kUSBProductID),
                         CFNumberCreate(kCFAllocatorDefault,
                                        kCFNumberSInt32Type, &mFilter.pid));
    IOServiceGetMatchingServices(kIOMasterPortDefault,
                                 mMatchingDictionary, &iterator);

    usbRef = IOIteratorNext(iterator);

}

qint32 QUsbDevice::open()
{
    IOCFPlugInInterface** plugin;
    io_iterator_t iterator = 0;
    io_service_t usbRef = 0;
    qint32 ret = 0;
    qint32 score = 0;
    CFDictionaryAddValue(mMatchingDictionary,
                         CFSTR(kUSBVendorID),
                         CFNumberCreate(kCFAllocatorDefault,
                                        kCFNumberSInt32Type, &mFilter.vid));
    CFDictionaryAddValue(mMatchingDictionary,
                         CFSTR(kUSBProductID),
                         CFNumberCreate(kCFAllocatorDefault,
                                        kCFNumberSInt32Type, &mFilter.pid));
    IOServiceGetMatchingServices(kIOMasterPortDefault,
                                 mMatchingDictionary, &iterator);

    usbRef = IOIteratorNext(iterator);

    if (usbRef == 0)
    {
        qWarning("Device not found");
        return -1;
    }

    IOObjectRelease(iterator);
    IOCreatePlugInInterfaceForService(usbRef, kIOUSBDeviceUserClientTypeID,
                                      kIOCFPlugInInterfaceID, &plugin, &score);
    IOObjectRelease(usbRef);
    ret = (*plugin)->QueryInterface(plugin,
                              CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID300),
                              (LPVOID)&mUsbInterface);
    (*plugin)->Release(plugin);

    if (ret || !mUsbInterface)
    {
        qWarning("Could not get interface (error: %x)\n", ret);
        return -2;
    }

    ret = (*mUsbDevice)->USBDeviceOpen(mUsbDevice);
    if (ret == kIOReturnSuccess)
    {
        // set first configuration as active
        ret = (*mUsbDevice)->GetConfigurationDescriptorPtr(mUsbDevice, 0, &config);
        if (ret != kIOReturnSuccess)
        {
            qWarning("Could not set active configuration (error: %x)\n", ret);
            return -3;
        }
        (*mUsbDevice)->SetConfiguration(mUsbDevice, config->bConfigurationValue);
    }
    else if (ret == kIOReturnExclusiveAccess)
    {
        // this is not a problem as we can still do some things
    }
    else
    {
        qWarning("Could not open device (error: %x)\n", ret);
        return -4;
    }

    ret = (*mUsbInterface)->USBInterfaceOpen(mUsbInterface);
    if (ret != kIOReturnSuccess)
    {
        qWarning("Could not open interface (error: %x)\n", ret);
        return -5;
    }

    return 0;
}

void QUsbDevice::close()
{
    if (mUsbInterface)
        (*mUsbInterface)->USBInterfaceClose(mUsbInterface);
    if (mUsbDevice)
        (*mUsbDevice)->USBDeviceClose(mUsbDevice);
}

void QUsbDevice::setDebug(bool enable)
{

}

void QUsbDevice::printUsbError(int error_code)
{

}

void QUsbDevice::flush()
{

}

qint32 QUsbDevice::read(QByteArray* buf, quint32 maxSize)
{
    quint32 numBytes = 0;
    qint32 ret = (*usbInterface)->ReadPipe(usbInterface, 2, in, &numBytes);

    if (ret == kIOReturnSuccess)
    {
        printf("Read %d bytes\n", numBytes);
        return numBytes;
    }
    else
    {
        printf("Read failed (error: %x)\n", ret);
        return -1;
    }
}

qint32 QUsbDevice::write(const QByteArray* buf, quint32 maxSize)
{
    qint32 count = 0;
    count = (*mUsbInterface)->WritePipe(mUsbInterface, mConfig.writeEp, buf->constData(), maxSize);

    return count;
}
