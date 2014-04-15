#include "qwinusb.h"

QUsb::QUsb(QBaseUsb *parent) :
    QBaseUsb(parent)
{
    this->mDevHandle = INVALID_HANDLE_VALUE;
    this->mUsbHandle = INVALID_HANDLE_VALUE;
    this->mDevSpeed = 0;
}

QUsb::~QUsb()
{
    this->close();
}

qint32 QUsb::open()
{
    if (!GetDeviceHandle(this->mGuidDeviceInterface, &this->mDevHandle)) return -1;
    else if (!GetWinUSBHandle(this->mDevHandle, &this->mUsbHandle)) return -2;
    else if (!GetUSBDeviceSpeed(this->mUsbHandle, &this->mDevSpeed)) return -3;
    else if (!QueryDeviceEndpoints(this->mUsbHandle, &this->mPipeId)) return -4;

    if (!WinUsb_SetPipePolicy(this->mUsbHandle, this->mReadEp, PIPE_TRANSFER_TIMEOUT, sizeof(ulong), &this->mTimeout)) {
        qWarning("Error WinUsb_SetPipePolicy: %d.\n", GetLastError()); return -5; }
    if (!WinUsb_SetPipePolicy(this->mUsbHandle, this->mWriteEp, PIPE_TRANSFER_TIMEOUT, sizeof(ulong), &this->mTimeout)) {
        qWarning("Error WinUsb_SetPipePolicy: %d.\n", GetLastError()); return -6; }

    bool enable = true; // ms
    if (!WinUsb_SetPipePolicy(this->mUsbHandle, this->mReadEp, IGNORE_SHORT_PACKETS, sizeof(enable), &enable)) {
        qWarning("Error WinUsb_SetPipePolicy: %d.\n", GetLastError()); return -7; }

    return 0;
}

void QUsb::close()
{
    CloseHandle(this->mDevHandle);
    WinUsb_Free(this->mUsbHandle);
}

qint32 QUsb::read(QByteArray *buf, quint32 bytes)
{
    PrintFuncName();
    if (this->mUsbHandle == INVALID_HANDLE_VALUE || !this->mReadEp)
    {
        return -1;
    }
    bool bResult = true;
    ulong cbRead = 0;
    uchar *buffer = new uchar[bytes];
    bResult = WinUsb_ReadPipe(this->mUsbHandle, this->mReadEp, buffer, bytes, &cbRead, 0);
    // we clear the buffer.
    buf->clear();

    if (!bResult) {
        PrintUsbError("WinUsb_ReadPipe");
        delete buffer;
        return -1;
    }

    else if (cbRead > 0){

        buf->append((const char *)buffer, cbRead);

        if (this->mDebug) {
            QString data, s;
            for (quint32 i = 0; i < cbRead; i++) {
                data.append(s.sprintf("%02X",buffer[i])+":");
            }
            data.remove(data.size()-1, 1); //remove last colon
            qDebug() << "Received" << cbRead << "of" << bytes << "Bytes:" << data;
        }
    }
    delete buffer;

    return cbRead;
}

qint32 QUsb::write(QByteArray *buf, quint32 bytes)
{
    PrintFuncName();
    if (this->mUsbHandle==INVALID_HANDLE_VALUE || !this->mWriteEp)
    {
        return -1;
    }

    ulong cbSent = 0;
    bool bResult = WinUsb_WritePipe(this->mUsbHandle, this->mWriteEp, (uchar*)buf->data(), bytes, &cbSent, 0);

    if (this->mDebug) {
        QString data, s;
        for (int i = 0; i < buf->size(); i++) {
            data.append(s.sprintf("%02X",(uchar)buf->at(i))+":");
        }
        data.remove(data.size()-1, 1); //remove last colon
        qDebug() << "Sent" << cbSent << "/" << buf->size() << "bytes:" << data;
    }
    if (!bResult) {
        PrintUsbError("WinUsb_WritePipe");
        return -1;
    }
    return cbSent;
}

bool QUsb::setGuid(QString guid)
{
    bool check[11];
    QString cleaned = guid.remove('-');
    GUID tmp =
    {
        cleaned.mid(0, 7).toInt(&check[0], 16),
        cleaned.mid(8, 11).toInt(&check[1], 16),
        cleaned.mid(12, 15).toInt(&check[2], 16),
        {
            cleaned.mid(16, 17).toInt(&check[3], 16),
            cleaned.mid(18, 19).toInt(&check[4], 16),
            cleaned.mid(20, 21).toInt(&check[5], 16),
            cleaned.mid(22, 23).toInt(&check[6], 16),
            cleaned.mid(24, 25).toInt(&check[7], 16),
            cleaned.mid(26, 27).toInt(&check[8], 16),
            cleaned.mid(29, 29).toInt(&check[9], 16),
            cleaned.mid(30, 31).toInt(&check[10], 16)
        }
    };

    for (quint8 i = 0; i < sizeof(check); i++) {

        if (!check[i]) {
            qWarning() << "Failed to set Device GUID!";
            return false;
        }
    }
    this->mGuid = tmp;
    return true;
}

bool QUsb::GetDeviceHandle(GUID guidDeviceInterface, PHANDLE hDeviceHandle)
{
    PrintFuncName();
    if (guidDeviceInterface==GUID_NULL)
    {
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
        PrintUsbError("SetupDiGetClassDevs");
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
            break;
        }

        //Check for some other error
        if (!bResult)
        {
            PrintUsbError("SetupDiEnumDeviceInterfaces");
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
                PrintUsbError("SetupDiEnumDeviceInterfaces");
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
            PrintUsbError("SetupDiGetDeviceInterfaceDetail");
            goto done;
        }

        //copy device path

        size_t nLength = wcslen (pInterfaceDetailData->DevicePath) + 1;
        lpDevicePath = (TCHAR *) LocalAlloc (LPTR, nLength * sizeof(TCHAR));
        StringCchCopy(lpDevicePath, nLength, pInterfaceDetailData->DevicePath);
        lpDevicePath[nLength-1] = 0;

        if (this->mDebug) qDebug("Device path:  %s\n", lpDevicePath);

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
        PrintUsbError("CreateFile");
        goto done;
    }

    done:
        LocalFree(lpDevicePath);
        LocalFree(pInterfaceDetailData);
        bResult = SetupDiDestroyDeviceInfoList(hDeviceInfo);

    return bResult;
}

bool QUsb::GetWinUSBHandle(HANDLE hDeviceHandle, PWINUSB_INTERFACE_HANDLE phWinUSBHandle)
{
    PrintFuncName();
    if (hDeviceHandle == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    if(!WinUsb_Initialize(hDeviceHandle, phWinUSBHandle))
    {
        //Error.
        PrintUsbError("WinUsb_Initialize");
        return false;
    }

    return true;
}

bool QUsb::GetUSBDeviceSpeed(WINUSB_INTERFACE_HANDLE hWinUSBHandle, quint8 *pDeviceSpeed)
{
    PrintFuncName();
    if (!pDeviceSpeed || hWinUSBHandle==INVALID_HANDLE_VALUE)
    {
        return false;
    }

    ulong length = sizeof(quint8);

    if(!WinUsb_QueryDeviceInformation(hWinUSBHandle, DEVICE_SPEED, &length, pDeviceSpeed))
    {
        PrintUsbError("Error getting device speed");
        return false;
    }

    if(*pDeviceSpeed == LowSpeed)
    {
        if (this->mDebug) qDebug("Device speed: %d (Low speed).\n", *pDeviceSpeed);
        return true;
    }
    else if(*pDeviceSpeed == FullSpeed)
    {
        if (this->mDebug) qDebug("Device speed: %d (Full speed).\n", *pDeviceSpeed);
        return true;
    }
    else if(*pDeviceSpeed == HighSpeed)
    {
        if (this->mDebug) qDebug("Device speed: %d (High speed).\n", *pDeviceSpeed);
        return true;
    }
    return false;
}

bool QUsb::QueryDeviceEndpoints(WINUSB_INTERFACE_HANDLE hWinUSBHandle, QUsb::PIPE_ID *pipeid)
{
    PrintFuncName();
    if (hWinUSBHandle==INVALID_HANDLE_VALUE)
    {
        return false;
    }

    bool bResult = true;

    USB_INTERFACE_DESCRIPTOR InterfaceDescriptor;
    ZeroMemory(&InterfaceDescriptor, sizeof(USB_INTERFACE_DESCRIPTOR));

    WINUSB_PIPE_INFORMATION  Pipe;
    ZeroMemory(&Pipe, sizeof(WINUSB_PIPE_INFORMATION));

    bResult = WinUsb_QueryInterfaceSettings(hWinUSBHandle, 0, &InterfaceDescriptor);

    if (bResult)
    {
        for (int index = 0; index < InterfaceDescriptor.bNumEndpoints; index++)
        {
            bResult = WinUsb_QueryPipe(hWinUSBHandle, 0, index, &Pipe);

            if (bResult)
            {
                if (Pipe.PipeType == UsbdPipeTypeControl)
                {
                    if (this->mDebug) qDebug("Endpoint index: %d Pipe type: Control Pipe ID: 0x%02x.\n", index, Pipe.PipeId);
                }
                if (Pipe.PipeType == UsbdPipeTypeIsochronous)
                {
                    if (this->mDebug) qDebug("Endpoint index: %d Pipe type: Isochronous Pipe ID: 0x%02x.\n", index, Pipe.PipeId);
                }
                if (Pipe.PipeType == UsbdPipeTypeBulk)
                {
                    if (USB_ENDPOINT_DIRECTION_IN(Pipe.PipeId))
                    {
                        if (this->mDebug) qDebug("Bulk IN Endpoint index: %d Pipe type: Bulk Pipe ID: 0x%02x.\n", index, Pipe.PipeId);
                        pipeid->PipeInId = Pipe.PipeId;
                    }
                    if (USB_ENDPOINT_DIRECTION_OUT(Pipe.PipeId))
                    {
                        if (this->mDebug) qDebug("Bulk OUT Endpoint index: %d Pipe type: Bulk Pipe ID: 0x%02x.\n", index, Pipe.PipeId);
                        pipeid->PipeOutId = Pipe.PipeId;
                    }

                }
                if (Pipe.PipeType == UsbdPipeTypeInterrupt)
                {
                    if (this->mDebug) qDebug("Endpoint index: %d Pipe type: Interrupt Pipe ID: 0x%02x.\n", index, Pipe.PipeId);
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

void QUsb::PrintUsbError(const QString &func)
{
    const quint32 err = GetLastError();
    switch (err) {

    case 121:
        qWarning() << func << "ERROR_SEM_TIMEOUT" << err;
        break;
    case 997:
        qWarning() << func << "ERROR_IO_PENDING" << err;
        break;
    default:
        qWarning() << func << "Error id:" << err;
        break;
    }
}
