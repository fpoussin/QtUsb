#ifndef QWINUSB_H
#define QWINUSB_H

#include <QDebug>
#include "qusb_compat.h"
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

#ifdef interface
    #undef interface // combaseapi.h
#endif

namespace QtUsb {
    const GUID WINUSB_DEV_GUID = {0xA5DCBF10L, 0x6530, 0x11D2, 0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED}; /**< TODO: describe */
}

/**
 * @brief
 *
 */
class QUSBSHARED_EXPORT QUsbDevice : public QBaseUsbDevice
{
    Q_OBJECT

    /**
     * @brief
     *
     */
    struct PIPE_ID
    {
        uchar  pipeInId; /**< TODO: describe */
        uchar  pipeOutId; /**< TODO: describe */
    };
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
     * @return QtUsb::FilterList
     */
    static QtUsb::FilterList getAvailableDevices(void);
    /**
     * @brief
     *
     * @param str
     * @param guid
     * @return bool
     */
    static bool guidFromString(const QString &str, GUID* guid);
    /**
     * @brief
     *
     * @param guid
     * @return bool
     */
    bool setGuid(const QString &guid);
    /**
     * @brief
     *
     * @param guid
     * @return bool
     */
    bool setGuid(const GUID &guid);
    /**
     * @brief
     *
     */
    ~QUsbDevice();
    
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
     * @param timeout
     */
    void setTimeout(quint16 timeout);

private:
    /**
     * @brief
     *
     * @param guidDeviceInterface
     * @param hDeviceHandle
     * @return bool
     */
    bool getDeviceHandle(GUID guidDeviceInterface, PHANDLE hDeviceHandle);
    /**
     * @brief
     *
     * @param hDeviceHandle
     * @param phWinUSBHandle
     * @return bool
     */
    bool getWinUSBHandle(HANDLE hDeviceHandle, PWINUSB_INTERFACE_HANDLE phWinUSBHandle);
    /**
     * @brief
     *
     * @param hWinUSBHandle
     * @param pDeviceSpeed
     * @return bool
     */
    bool getUSBDeviceSpeed(WINUSB_INTERFACE_HANDLE hWinUSBHandle, quint8 *pDeviceSpeed);
    /**
     * @brief
     *
     * @param hWinUSBHandle
     * @param pipeId
     * @return bool
     */
    bool queryDeviceEndpoints(WINUSB_INTERFACE_HANDLE hWinUSBHandle, PIPE_ID* pipeId);
    /**
     * @brief
     *
     * @param func
     */
    void printUsbError(const QString& func);

    GUID mGuid; /**< TODO: describe */
    HANDLE mDevHandle; /**< TODO: describe */
    WINUSB_INTERFACE_HANDLE mUsbHandle; /**< TODO: describe */
    uchar mDevSpeed; /**< TODO: describe */
    PIPE_ID mPipeId; /**< TODO: describe */
};

#endif // QWINUSB_H
