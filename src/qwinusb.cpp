#include "qwinusb.h"

QUsbDevice::QUsbDevice(QBaseUsbDevice *parent) :
    QBaseUsbDevice(parent)
{
    mDevHandle = INVALID_HANDLE_VALUE;
    mUsbHandle = INVALID_HANDLE_VALUE;
    mDevSpeed = 0;
}

QList<QtUsb::DeviceFilter> QUsbDevice::getAvailableDevices()
{
    QList<QtUsb::DeviceFilter> list;
    GUID usbGuid;
    this->guidFromString(QtUsb::WINUSB_GUID, &usbGuid);

    HDEVINFO                         hDevInfo;
    SP_DEVICE_INTERFACE_DATA         devIntfData;
    PSP_DEVICE_INTERFACE_DETAIL_DATA devIntfDetailData;
    SP_DEVINFO_DATA                  devData;

    DWORD dwSize, dwMemberIdx;

    hDevInfo = SetupDiGetClassDevs(
            &usbGuid, NULL, 0, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);


    if (hDevInfo != INVALID_HANDLE_VALUE)
        {
            devIntfData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
            dwMemberIdx = 0;

            SetupDiEnumDeviceInterfaces(hDevInfo, NULL, &usbGuid,
                dwMemberIdx, &devIntfData);

            while(GetLastError() != ERROR_NO_MORE_ITEMS)
            {
                devData.cbSize = sizeof(devData);
                SetupDiGetDeviceInterfaceDetail(
                      hDevInfo, &devIntfData, NULL, 0, &dwSize, NULL);

                devIntfDetailData = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwSize);
                devIntfDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

                if (SetupDiGetDeviceInterfaceDetail(hDevInfo, &devIntfData,
                    devIntfDetailData, dwSize, &dwSize, &devData))
                {
                    QString pathStr(devIntfDetailData);
                    QtUsb::DeviceFilter filter;

                    int vidFrom = pathStr.indexOf("vid_")+4;
                    QString vidStr = pathStr.mid(vidFrom, 4);

                    int pidFrom = pathStr.indexOf("pid_")+4;
                    QString pidStr = pathStr.mid(pidFrom, 4);

                    bool ok[2];
                    uint vid = vidStr.toUInt(ok[1], 16);
                    uint pid = pidStr.toUInt(ok[2], 16);

                    if (ok[1] && ok[2])
                    {
                        filter.pid = pid;
                        filter.vid = vid;
                        list.append(filter);
                    }
                }

                HeapFree(GetProcessHeap(), 0, devIntfDetailData);

                // Continue looping
                SetupDiEnumDeviceInterfaces(
                    hDevInfo, NULL, &usbGuid, ++dwMemberIdx, &devIntfData);
            }

            SetupDiDestroyDeviceInfoList(hDevInfo);
        }

    return list;
}

bool QUsbDevice::guidFromString(const QString& str, GUID *guid)
{
    bool check[11];
    QString cleaned(str);
    cleaned.remove('-');
    GUID tmp =
    {
        cleaned.mid(0, 8).toULong(&check[0], 16),
        cleaned.mid(8, 4).toUInt(&check[1], 16)&0xFFFF,
        cleaned.mid(12, 4).toUInt(&check[2], 16)&0xFFFF,
        {
            cleaned.mid(16, 2).toUInt(&check[3], 16)&0xFF,
            cleaned.mid(18, 2).toUInt(&check[4], 16)&0xFF,
            cleaned.mid(20, 2).toUInt(&check[5], 16)&0xFF,
            cleaned.mid(22, 2).toUInt(&check[6], 16)&0xFF,
            cleaned.mid(24, 2).toUInt(&check[7], 16)&0xFF,
            cleaned.mid(26, 2).toUInt(&check[8], 16)&0xFF,
            cleaned.mid(28, 2).toUInt(&check[9], 16)&0xFF,
            cleaned.mid(30, 2).toUInt(&check[10], 16)&0xFF
        }
    };

    for (quint8 i = 0; i < sizeof(check); i++) {

        if (!check[i]) {
            qWarning() << "Failed to set Device GUID" << guid << "at" << i;
            return false;
        }
    }

    guid = tmp;
    return true;
}

QUsbDevice::~QUsbDevice()
{
    this->close();
}

bool QUsbDevice::open(QIODevice::OpenMode mode)
{
    (void) mode;
    return this->open() == 0;
}

qint32 QUsbDevice::open()
{
    if (mConnected)
        return -1;

    if (!getDeviceHandle(mGuidDeviceInterface, &mDevHandle)) return -1;
    else if (!getWinUSBHandle(mDevHandle, &mUsbHandle)) return -2;
    else if (!getUSBDeviceSpeed(mUsbHandle, &mDevSpeed)) return -3;
    else if (!queryDeviceEndpoints(mUsbHandle, &mPipeId)) return -4;

    ulong timeout = mTimeout; /* SetPipePolicy requires an unsigned long */
    if (!WinUsb_SetPipePolicy(mUsbHandle, mConfig.readEp, PIPE_TRANSFER_TIMEOUT, sizeof(timeout), &timeout)) {
        qWarning("Error WinUsb_SetPipePolicy: %d.\n", GetLastError()); return -5; }
    if (!WinUsb_SetPipePolicy(mUsbHandle, mConfig.writeEp, PIPE_TRANSFER_TIMEOUT, sizeof(timeout), &timeout)) {
        qWarning("Error WinUsb_SetPipePolicy: %d.\n", GetLastError()); return -6; }

    bool enable = true;
    if (!WinUsb_SetPipePolicy(mUsbHandle, mConfig.readEp, IGNORE_SHORT_PACKETS, sizeof(enable), &enable)) {
        qWarning("Error WinUsb_SetPipePolicy: %d.\n", GetLastError()); return -7; }

    mConnected = true;

    return 0;
}

void QUsbDevice::close()
{
    if (!mConnected)
        return;

    if (mDevHandle != INVALID_HANDLE_VALUE)
        CloseHandle(mDevHandle);
    if (mUsbHandle != INVALID_HANDLE_VALUE)
        WinUsb_Free(mUsbHandle);

    mDevHandle = INVALID_HANDLE_VALUE;
    mUsbHandle = INVALID_HANDLE_VALUE;

    mConnected = false;
}

bool QUsbDevice::setGuid(const QString &guid)
{
    return this->guidFromString(guid, &mGuidDeviceInterface);
}

bool QUsbDevice::setGuid(const GUID &guid)
{
    mGuidDeviceInterface = guid;
    return true;
}

bool QUsbDevice::getDeviceHandle(GUID guidDeviceInterface, PHANDLE hDeviceHandle)
{
    PrintFuncName();
    if (guidDeviceInterface == GUID_NULL)
    {
        printUsbError("GUID_NULL");
        return false;
    }

    bool bResult = true;
    HDEVINFO hDeviceInfo;
    SP_DEVINFO_DATA DeviceInfoData;

    SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
    PSP_DEVICE_INTERFACE_DETAIL_DATA pInterfaceDetailData = NULL;

    ulong requiredLength = 0;

    LPTSTR lpDevicePath = NULL;

    qint32 index = 0;

    // Get information about all the installed devices for the specified
    // device interface class.
    hDeviceInfo = SetupDiGetClassDevs(
        &guidDeviceInterface,
        NULL,
        NULL,
        DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

    if (hDeviceInfo == INVALID_HANDLE_VALUE)
    {
        // ERROR
        printUsbError("SetupDiGetClassDevs");
        goto done;
    }

    //Enumerate all the device interfaces in the device information set.
    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    for (index = 0; SetupDiEnumDeviceInfo(hDeviceInfo, index, &DeviceInfoData); index++)
    {
        //Reset for this iteration
        if (lpDevicePath)
        {
            LocalFree(lpDevicePath);
        }
        if (pInterfaceDetailData)
        {
            LocalFree(pInterfaceDetailData);
        }

        deviceInterfaceData.cbSize = sizeof(SP_INTERFACE_DEVICE_DATA);

        //Get information about the device interface.
        bResult = SetupDiEnumDeviceInterfaces(
           hDeviceInfo,
           &DeviceInfoData,
           &guidDeviceInterface,
           0,
           &deviceInterfaceData);

        // Check if last item
        if (GetLastError () == ERROR_NO_MORE_ITEMS)
        {
            printUsbError("ERROR_NO_MORE_ITEMS");
            break;
        }

        //Check for some other error
        if (!bResult)
        {
            printUsbError("SetupDiEnumDeviceInterfaces");
            goto done;
        }

        //Interface data is returned in SP_DEVICE_INTERFACE_DETAIL_DATA
        //which we need to allocate, so we have to call this function twice.
        //First to get the size so that we know how much to allocate
        //Second, the actual call with the allocated buffer

        bResult = SetupDiGetDeviceInterfaceDetail(
            hDeviceInfo,
            &deviceInterfaceData,
            NULL, 0,
            &requiredLength,
            NULL);


        //Check for some other error
        if (!bResult)
        {
            if ((ERROR_INSUFFICIENT_BUFFER==GetLastError()) && (requiredLength>0))
            {
                //we got the size, allocate buffer
                pInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)LocalAlloc(LPTR, requiredLength);

                if (!pInterfaceDetailData)
                {
                    // ERROR
                    qWarning("Error allocating memory for the device detail buffer.\n");
                    goto done;
                }
            }
            else
            {
                printUsbError("SetupDiEnumDeviceInterfaces");
                goto done;
            }
        }

        //get the interface detailed data
        pInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

        //Now call it with the correct size and allocated buffer
        bResult = SetupDiGetDeviceInterfaceDetail(
                hDeviceInfo,
                &deviceInterfaceData,
                pInterfaceDetailData,
                requiredLength,
                NULL,
                &DeviceInfoData);

        //Check for some other error
        if (!bResult)
        {
            printUsbError("SetupDiGetDeviceInterfaceDetail");
            goto done;
        }

        //copy device path

        size_t nLength = wcslen (pInterfaceDetailData->DevicePath) + 1;
        lpDevicePath = (TCHAR *) LocalAlloc (LPTR, nLength * sizeof(TCHAR));
        StringCchCopy(lpDevicePath, nLength, pInterfaceDetailData->DevicePath);
        lpDevicePath[nLength-1] = 0;

        if (mDebug) qDebug("Device path:  %s\n", lpDevicePath);

    }

    if (!lpDevicePath)
    {
        //Error.
        qWarning("Error %d.", GetLastError());
        goto done;
    }

    //Open the device
    *hDeviceHandle = CreateFile (
        lpDevicePath,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_OVERLAPPED,
        NULL);

    if (*hDeviceHandle == INVALID_HANDLE_VALUE)
    {
        //Error.
        printUsbError("CreateFile");
        goto done;
    }

    done:
        LocalFree(lpDevicePath);
        LocalFree(pInterfaceDetailData);
        bResult = SetupDiDestroyDeviceInfoList(hDeviceInfo);

    return bResult;
}

bool QUsbDevice::getWinUSBHandle(HANDLE hDeviceHandle, PWINUSB_INTERFACE_HANDLE phWinUSBHandle)
{
    PrintFuncName();
    if (hDeviceHandle == INVALID_HANDLE_VALUE)
    {
        printUsbError("INVALID_HANDLE_VALUE");
        return false;
    }

    if(!WinUsb_Initialize(hDeviceHandle, phWinUSBHandle))
    {
        //Error.
        printUsbError("WinUsb_Initialize");
        return false;
    }

    return true;
}

bool QUsbDevice::getUSBDeviceSpeed(WINUSB_INTERFACE_HANDLE hWinUSBHandle, quint8 *pDeviceSpeed)
{
    PrintFuncName();
    if (!pDeviceSpeed || hWinUSBHandle==INVALID_HANDLE_VALUE)
    {
        return false;
    }

    ulong length = sizeof(quint8);

    if(!WinUsb_QueryDeviceInformation(hWinUSBHandle, DEVICE_SPEED, &length, pDeviceSpeed))
    {
        printUsbError("Error getting device speed");
        return false;
    }

    if(*pDeviceSpeed == LowSpeed)
    {
        if (mDebug) qDebug("Device speed: %d (Low speed).\n", *pDeviceSpeed);
        this->mSpd = QtUsb::lowSpeed;
        return true;
    }
    else if(*pDeviceSpeed == FullSpeed)
    {
        if (mDebug) qDebug("Device speed: %d (Full speed).\n", *pDeviceSpeed);
        this->mSpd = QtUsb::fullSpeed;
        return true;
    }
    else if(*pDeviceSpeed == HighSpeed)
    {
        if (mDebug) qDebug("Device speed: %d (High speed).\n", *pDeviceSpeed);
        this->mSpd = QtUsb::highSpeed;
        return true;
    }
    return false;

}

bool QUsbDevice::queryDeviceEndpoints(WINUSB_INTERFACE_HANDLE hWinUSBHandle, QUsbDevice::PIPE_ID *pipeId)
{
    PrintFuncName();
    if (hWinUSBHandle==INVALID_HANDLE_VALUE)
    {
        return false;
    }

    bool bResult = true;

    USB_INTERFACE_DESCRIPTOR InterfaceDescriptor;
    ZeroMemory(&InterfaceDescriptor, sizeof(USB_INTERFACE_DESCRIPTOR));

    WINUSB_PIPE_INFORMATION  pipe;
    ZeroMemory(&pipe, sizeof(WINUSB_PIPE_INFORMATION));

    bResult = WinUsb_QueryInterfaceSettings(hWinUSBHandle, 0, &InterfaceDescriptor);

    if (bResult)
    {
        for (int index = 0; index < InterfaceDescriptor.bNumEndpoints; index++)
        {
            bResult = WinUsb_QueryPipe(hWinUSBHandle, 0, index, &pipe);

            if (bResult)
            {
                if (pipe.PipeType == UsbdPipeTypeControl)
                {
                    if (mDebug) qDebug("Endpoint index: %d Pipe type: Control Pipe ID: 0x%02x.\n", index, pipe.PipeId);
                }
                if (pipe.PipeType == UsbdPipeTypeIsochronous)
                {
                    if (mDebug) qDebug("Endpoint index: %d Pipe type: Isochronous Pipe ID: 0x%02x.\n", index, pipe.PipeId);
                }
                if (pipe.PipeType == UsbdPipeTypeBulk)
                {
                    if (USB_ENDPOINT_DIRECTION_IN(pipe.PipeId))
                    {
                        if (mDebug) qDebug("Bulk IN Endpoint index: %d Pipe type: Bulk Pipe ID: 0x%02x.\n", index, pipe.PipeId);
                        pipeId->pipeInId = pipe.PipeId;
                    }
                    if (USB_ENDPOINT_DIRECTION_OUT(pipe.PipeId))
                    {
                        if (mDebug) qDebug("Bulk OUT Endpoint index: %d Pipe type: Bulk Pipe ID: 0x%02x.\n", index, pipe.PipeId);
                        pipeId->pipeOutId = pipe.PipeId;
                    }

                }
                if (pipe.PipeType == UsbdPipeTypeInterrupt)
                {
                    if (mDebug) qDebug("Endpoint index: %d Pipe type: Interrupt Pipe ID: 0x%02x.\n", index, pipe.PipeId);
                }
            }
            else
            {
                continue;
            }
        }
    }
    else
    {
        return false;
    }
    return true;
}

void QUsbDevice::printUsbError(const QString &func)
{
    const quint32 err = GetLastError();
    switch (err)
    {
        case ERROR_INVALID_HANDLE:
            qWarning() << func << "ERROR_INVALID_HANDLE" << err;
            break;
        case ERROR_SEM_TIMEOUT:
            qWarning() << func << "ERROR_SEM_TIMEOUT" << err;
            break;
        case ERROR_IO_PENDING:
            qWarning() << func << "ERROR_IO_PENDING" << err;
            break;
        case ERROR_NOT_ENOUGH_MEMORY:
            qWarning() << func << "ERROR_NOT_ENOUGH_MEMORY" << err;
            break;
        default:
            qWarning() << func << "Error id:" << err;
            break;
    }
}

qint64 QUsbDevice::readData(char *data, qint64 maxSize)
{
    PrintFuncName();

    if (mUsbHandle == INVALID_HANDLE_VALUE
            || !mConfig.readEp
            || !mConnected)
    {
        return -1;
    }
    bool bResult = true;
    ulong cbRead = 0;
    bResult = WinUsb_ReadPipe(mUsbHandle, mConfig.readEp, data, maxSize, &cbRead, 0);

    if (!bResult) {
        printUsbError("WinUsb_ReadPipe");
        return -1;
    }

    else if (cbRead > 0){

        if (mDebug) {
            QString datastr, s;
            for (quint32 i = 0; i < cbRead; i++) {
                datastr.append(s.sprintf("%02X", data[i])+":");
            }
            datastr.remove(datastr.size()-1, 1); //remove last colon
            qDebug() << "Received" << cbRead << "of" << maxSize << "Bytes:" << datastr;
        }
    }

    return cbRead;
}

qint64 QUsbDevice::writeData(const char *data, qint64 maxSize)
{
    PrintFuncName();
    if (mUsbHandle==INVALID_HANDLE_VALUE
            || !mConfig.writeEp
            || !mConnected)
    {
        return -1;
    }

    ulong cbSent = 0;
    bool bResult = WinUsb_WritePipe(mUsbHandle, mConfig.writeEp, (uchar*)data, maxSize, &cbSent, 0);

    if (mDebug) {
        QString datastr, s;
        for (qint64 i = 0; i < maxSize; i++) {
            datastr.append(s.sprintf("%02X", data[i])+":");
        }
        datastr.remove(datastr.size()-1, 1); //remove last colon
        qDebug() << "Sent" << cbSent << "/" << maxSize << "bytes:" << datastr;
    }
    if (!bResult) {
        printUsbError("WinUsb_WritePipe");
        return -1;
    }
    return cbSent;
}
