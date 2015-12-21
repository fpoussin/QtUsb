#ifndef QIOUSB_H
#define QIOUSB_H

#include <QString>
#include <QDebug>
#include <QByteArray>
#include <QtEndian>
#include "qbaseusb.h"
#include "qusb_compat.h"

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/usb/IOUSBLib.h>
#include <IOKit/usb/USBSpec.h>

/**
 * @brief
 *
 */
class QUSBSHARED_EXPORT QUsbDevice : public QBaseUsbDevice
{
    Q_OBJECT

public:
    /**
     * @brief
     *
     * @param parent
     */
    explicit QUsbDevice(QBaseUsbDevice *parent = 0);
    /**
     * @brief
     *
     */
    ~QUsbDevice();

    /**
     * @brief
     *
     * @return QtUsb::FilterList
     */
    static QtUsb::FilterList getAvailableDevices(void);

public slots:
    /**
     * @brief
     *
     * @return qint32
     */
    qint32 open();
    /**
     * @brief
     *
     */
    void close();

    /**
     * @brief
     *
     */
    void flush();
    /**
     * @brief
     *
     * @param buf
     * @param maxSize
     * @return qint32
     */
    qint32 read(QByteArray* buf, quint32 maxSize);
    /**
     * @brief
     *
     * @param buf
     * @param maxSize
     * @return qint32
     */
    qint32 write(const QByteArray* buf, quint32 maxSize);

    /**
     * @brief
     *
     * @param enable
     */
    void setDebug(bool enable);

private slots:    

private:
    /**
     * @brief
     *
     * @param error_code
     */
    void printUsbError(int error_code);

    CFMutableDictionaryRef mMatchingDictionary;
    IOCFPlugInInterface** mPlugin;
    IOUSBDeviceInterface300** mUsbDevice;
    IOUSBConfigurationDescriptorPtr mConfig;
    IOUSBFindInterfaceRequest mInterfaceRequest;
    IOUSBInterfaceInterface300** mUsbInterface;

};
#endif // QIOUSB_H
