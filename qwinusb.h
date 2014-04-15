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
        uchar  PipeInId;
        uchar  PipeOutId;
    };
public:
    explicit QUsb(QBaseUsb *parent = 0);
    ~QUsb();
    
public slots:
    qint32 open();
    void close();
    qint32 read(QByteArray *buf, quint32 bytes);
    qint32 write(QByteArray *buf, quint32 bytes);
    bool setGuid(QString guid);

private:
    bool GetDeviceHandle(GUID guidDeviceInterface, PHANDLE hDeviceHandle);
    bool GetWinUSBHandle(HANDLE hDeviceHandle, PWINUSB_INTERFACE_HANDLE phWinUSBHandle);
    bool GetUSBDeviceSpeed(WINUSB_INTERFACE_HANDLE hWinUSBHandle, quint8 *pDeviceSpeed);
    bool QueryDeviceEndpoints(WINUSB_INTERFACE_HANDLE hWinUSBHandle, PIPE_ID* pipeid);
    void PrintUsbError(const QString& func);

    GUID mGuidDeviceInterface;
    HANDLE mDevHandle;
    WINUSB_INTERFACE_HANDLE mUsbHandle;
    uchar mDevSpeed;
    PIPE_ID mPipeId;
    GUID mGuid;
};

#endif // QWINUSB_H
