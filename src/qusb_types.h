#ifndef QUSB_TYPES_H
#define QUSB_TYPES_H

#include <QMetaType>

namespace QtUsb {

    const quint16 DefaultTimeout = 250; /**< TODO: describe */
    /**
     * @brief
     *
     */
    enum DeviceSpeed {unknownSpeed = -1, lowSpeed = 0, fullSpeed, highSpeed, superSpeed};
    /**
     * @brief
     *
     */
    enum DeviceStatus {deviceOK = 0, deviceBusy = -1, deviceNotFound = -2, devicePgmError = -3};

    /**
     * @brief
     *
     */
    typedef struct
    {
        quint8 readEp; /**< TODO: describe */
        quint8 writeEp; /**< TODO: describe */
        quint8 config; /**< TODO: describe */
        quint8 interface; /**< TODO: describe */
        quint8 alternate; /**< TODO: describe */

    /**
     * @brief
     *
     */
    } DeviceConfig;

    /**
     * @brief
     *
     */
    typedef struct
    {
        quint16 pid; /**< TODO: describe */
        quint16 vid; /**< TODO: describe */
        QString guid; /**< TODO: describe */
        DeviceConfig cfg; /**< TODO: describe */

    /**
     * @brief
     *
     */
    } DeviceFilter;

    /**
     * @brief
     *
     */
    typedef QList<DeviceFilter> FilterList;
    /**
     * @brief
     *
     */
    typedef QList<DeviceConfig> ConfigList;
}

Q_DECLARE_METATYPE(QtUsb::DeviceFilter)
Q_DECLARE_METATYPE(QtUsb::DeviceConfig)

Q_DECLARE_METATYPE(QtUsb::FilterList)
Q_DECLARE_METATYPE(QtUsb::ConfigList)

#endif // QUSB_TYPES_H
