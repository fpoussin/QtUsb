#ifndef QWINUSB_H
#define QWINUSB_H

#include <QObject>
#include <QDebug>
#include "compat.h"
#include "qbaseusb.h"

// Include Windows headers
#include <windows.h>
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

class QUsb : public QBaseUsb
{
    Q_OBJECT

    struct PIPE_ID
    {
        uchar  pipeInId;
        uchar  pipeOutId;
    };
public:
    explicit QUsb(QBaseUsb *parent = 0);
    ~QUsb();
    
public slots:
    qint32 open();
    void close();
    qint32 read(QByteArray *buf, quint32 bytes);
    qint32 write(QByteArray *buf, quint32 bytes);
    bool setGuid(QString &guid);

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
    GUID mGuid;
};

#endif // QWINUSB_H
