#ifndef QUSBTYPES_H
#define QUSBTYPES_H

namespace QtUsb {

    const quint16 DefaultTimeout = 250;

    enum Speed {unknownSpeed = -1, lowSpeed = 0, fullSpeed, highSpeed, superSpeed};

    enum DeviceStatus {deviceOK = 0, deviceBusy = -1, deviceNotFound = -2};

    typedef struct
    {
        quint16 pid;
        quint16 vid;
        QString guid;

    } UsbDeviceFilter;

    typedef struct
    {
        quint8 readEp;
        quint8 writeEp;
        quint8 config;
        quint8 interface;
        quint8 alternate;

    } UsbDeviceConfig;

    typedef QList<UsbDeviceFilter> UsbFilterList;
    typedef QList<UsbDeviceConfig> UsbConfigList;
}

#endif // QUSBTYPES_H
