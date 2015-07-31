#ifndef QWINUSB_H
#define QWINUSB_H

#include <QDebug>
#include "compat.h"
#include "qbaseusb.h"

// Include Windows headers
#include <qt_windows.h>
#include <stdio.h>
#include <tchar.h>
#include <strsafe.h>

// Include WinUSB headers
#include <winusb.h>
#include <Usb100.h>
#include <Setupapi.h>

// Linked libraries
#pragma comment (lib , "setupapi.lib" )
#pragma comment (lib , "winusb.lib" )

namespace QtUsb {
    const QString WINUSB_GUID = "DBCE1CD9-A320-4b51-A365-A0C3F3C5FB29";
}

class QUSBSHARED_EXPORT QUsbDevice : public QBaseUsbDevice
{
    Q_OBJECT

    struct PIPE_ID
    {
        uchar  pipeInId;
        uchar  pipeOutId;
    };
public:
    explicit QUsbDevice(QBaseUsbDevice *parent = 0);
    static QList<QtUsb::DeviceFilter> getAvailableDevices(void);
    static bool guidFromString(const QString &str, GUID* guid);
    ~QUsbDevice();
    
public slots:
    bool open(OpenMode mode);
    qint32 open();
    void close();
    bool setGuid(const QString &guid);
    bool setGuid(const GUID &guid);

    qint64 readData(char * data, qint64 maxSize);
    qint64 writeData(const char * data, qint64 maxSize);

private:
    bool getDeviceHandle(GUID guidDeviceInterface, PHANDLE hDeviceHandle);
    bool getWinUSBHandle(HANDLE hDeviceHandle, PWINUSB_INTERFACE_HANDLE phWinUSBHandle);
    bool getUSBDeviceSpeed(WINUSB_INTERFACE_HANDLE hWinUSBHandle, quint8 *pDeviceSpeed);
    bool queryDeviceEndpoints(WINUSB_INTERFACE_HANDLE hWinUSBHandle, PIPE_ID* pipeId);
    void printUsbError(const QString& func);

    GUID mGuidDeviceInterface;
    HANDLE mDevHandle;
    WINUSB_INTERFACE_HANDLE mUsbHandle;
    uchar mDevSpeed;
    PIPE_ID mPipeId;
};

#endif // QWINUSB_H
