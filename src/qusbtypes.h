#ifndef QUSBTYPES_H
#define QUSBTYPES_H

namespace QtUsb {

    const quint16 DefaultTimeout = 250;

    enum DeviceSpeed {unknownSpeed = -1, lowSpeed = 0, fullSpeed, highSpeed, superSpeed};

    enum DeviceStatus {deviceOK = 0, deviceBusy = -1, deviceNotFound = -2};

    typedef struct
    {
        quint16 pid;
        quint16 vid;
        QString guid;

    } DeviceFilter;

    typedef struct
    {
        quint8 readEp;
        quint8 writeEp;
        quint8 config;
        quint8 interface;
        quint8 alternate;

    } DeviceConfig;

    typedef QList<DeviceFilter> FilterList;
    typedef QList<DeviceConfig> ConfigList;


}

Q_DECLARE_METATYPE(QtUsb::DeviceFilter);
Q_DECLARE_METATYPE(QtUsb::DeviceConfig);

Q_DECLARE_METATYPE(QtUsb::FilterList);
Q_DECLARE_METATYPE(QtUsb::ConfigList);

#endif // QUSBTYPES_H
