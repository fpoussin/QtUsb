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
     * @brief Endpoint structure
     *
     */
    struct PIPE_ID
    {
        uchar  pipeInId; /**< Read endpoint */
        uchar  pipeOutId; /**< write endpoint */
    };
public:
    /**
     * @brief See base class
     *
     * @param parent
     */
    explicit QUsbDevice(QBaseUsbDevice *parent = 0);
    /**
     * @brief See base class
     *
     * @return QtUsb::FilterList
     */
    static QtUsb::FilterList getAvailableDevices(void);
    /**
     * @brief Set Guid object from string
     *
     * @param str String containing the Guid
     * @param guid GUID object pointer to write
     * @return bool true on success
     */
    static bool guidFromString(const QString &str, GUID* guid);
    /**
     * @brief Set Guid member object from string
     *
     * @param guid String containing the Guid
     * @return bool true on success
     */
    bool setGuid(const QString &guid);
    /**
     * @brief Set Guid member object
     *
     * @param guid GUID object to copy
     * @return bool true on success
     */
    bool setGuid(const GUID &guid);
    /**
     * @brief
     *
     */
    ~QUsbDevice();
    
public slots:
    /**
     * @brief See base class
     *
     * @return qint32
     */
    qint32 open();
    /**
     * @brief See base class
     *
     */
    void close();
    /**
     * @brief See base class
     *
     */
    void flush();
    /**
     * @brief See base class
     *
     * @param buf
     * @param maxSize
     * @return qint32
     */
    qint32 read(QByteArray* buf, quint32 maxSize);
    /**
     * @brief See base class
     *
     * @param buf
     * @param maxSize
     * @return qint32
     */
    qint32 write(const QByteArray* buf, quint32 maxSize);

    /**
     * @brief See base class
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
     * @brief Print errors to qWarning
     *
     * @param func
     */
    void printUsbError(const QString& func);

    GUID mGuid; /**< GUID structure */
    HANDLE mDevHandle; /**< Device Handle */
    WINUSB_INTERFACE_HANDLE mUsbHandle; /**< WinUSB interfance handle */
    uchar mDevSpeed; /**< Device speed */
    PIPE_ID mPipeId; /**< Device endpoints */
};

#endif // QWINUSB_H
