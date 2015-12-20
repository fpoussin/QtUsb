#ifndef QUSBMANAGER_H
#define QUSBMANAGER_H

#include <QThread>
#include <QList>
#include <QUsb>

/**
 * @brief
 *
 */
class QUSBSHARED_EXPORT QUsbManager : public QThread
{
    Q_OBJECT
public:
    /**
     * @brief
     *
     * @param parent
     */
    explicit QUsbManager(QObject *parent = 0);
    /**
     * @brief
     *
     */
    ~QUsbManager(void);

    /**
     * @brief
     *
     * @return QtUsb::FilterList
     */
    QtUsb::FilterList getPresentDevices(void);
    /**
     * @brief
     *
     * @param filter
     * @return bool
     */
    bool isPresent(const QtUsb::DeviceFilter &filter);

signals:
    /**
     * @brief
     *
     * @param filters
     */
    void deviceInserted(QtUsb::FilterList filters);
    /**
     * @brief
     *
     * @param filters
     */
    void deviceRemoved(QtUsb::FilterList filters);

public slots:
    /**
     * @brief
     *
     * @param filter
     * @return bool
     */
    bool addDevice(const QtUsb::DeviceFilter &filter);
    /**
     * @brief
     *
     * @param filter
     * @return bool
     */
    bool removeDevice(const QtUsb::DeviceFilter &filter);
    /**
     * @brief
     *
     * @param filter
     * @param list
     * @return int
     */
    int findDevice(const QtUsb::DeviceFilter &filter, const QtUsb::FilterList &list);

    /**
     * @brief
     *
     * @param dev
     * @param filter
     * @param config
     * @return QtUsb::DeviceStatus
     */
    QtUsb::DeviceStatus openDevice(QUsbDevice* dev, const QtUsb::DeviceFilter &filter, const QtUsb::DeviceConfig &config);
    /**
     * @brief
     *
     * @param dev
     * @return QtUsb::DeviceStatus
     */
    QtUsb::DeviceStatus closeDevice(QUsbDevice* dev);

protected slots:
    /**
     * @brief
     *
     * @param list
     */
    void monitorDevices(const QtUsb::FilterList &list);
    /**
     * @brief
     *
     */
    void run(void);

protected:
    bool mStop; /**< TODO: describe */
    QList<QUsbDevice*> mUsedDeviceList; /**< TODO: describe */
    QtUsb::FilterList mFilterList; /**< TODO: describe */
    QtUsb::FilterList mSystemList; /**< TODO: describe */
};

#endif // QUSBMANAGER_H
