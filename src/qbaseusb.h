#ifndef QBASEUSB_H
#define QBASEUSB_H

#include <QString>
#include <QDebug>
#include <QList>
#include "qusb_global.h"
#include "qusb_types.h"

/**
 * @brief
 *
 */
class QUSBSHARED_EXPORT QBaseUsbDevice : public QObject
{
    Q_OBJECT
    
public:
    /**
     * @brief
     *
     * @param parent
     */
    QBaseUsbDevice(QObject *parent = 0);
    /**
     * @brief
     *
     */
    virtual ~QBaseUsbDevice();

    /**
     * @brief
     *
     * @param enable
     */
    void setDebug(bool enable) { mDebug = enable; }
    /**
     * @brief
     *
     * @param filter
     */
    void setFilter(const QtUsb::DeviceFilter& filter) { mFilter = filter; }
    /**
     * @brief
     *
     * @param config
     */
    void setConfig(const QtUsb::DeviceConfig& config) { mConfig = config; }

    /**
     * @brief
     *
     * @return QtUsb::DeviceFilter
     */
    QtUsb::DeviceFilter getFilter(void) { return mFilter; }
    /**
     * @brief
     *
     * @return QtUsb::DeviceConfig
     */
    QtUsb::DeviceConfig getConfig(void) { return mConfig; }

    /**
     * @brief
     *
     * @return quint16
     */
    quint16 getPid(void) { return mFilter.pid; }
    /**
     * @brief
     *
     * @return quint16
     */
    quint16 getVid(void) { return mFilter.vid; }
    /**
     * @brief
     *
     * @return QString
     */
    QString getGuid(void) { return mFilter.guid; }
    /**
     * @brief
     *
     * @return quint8
     */
    quint8 getReadEp(void) { return mConfig.readEp; }
    /**
     * @brief
     *
     * @return quint8
     */
    quint8 getWriteEp(void) { return mConfig.writeEp; }
    /**
     * @brief
     *
     * @return quint16
     */
    quint16 getTimeout(void) { return mTimeout; }

    /**
     * @brief
     *
     * @return QtUsb::DeviceSpeed
     */
    QtUsb::DeviceSpeed getSpeed(void) { return mSpd; }
    /**
     * @brief
     *
     * @return QString
     */
    QString getSpeedString(void);

    /**
     * @brief
     *
     * @return QtUsb::FilterList
     */
    static QtUsb::FilterList getAvailableDevices(void);

public slots:
    /**
     * @brief
     *
     * @return qint32
     */
    virtual qint32 open() = 0;
    /**
     * @brief
     *
     */
    virtual void close() = 0;
    /**
     * @brief
     *
     */
    virtual void flush() = 0;

    /**
     * @brief
     *
     * @param buf
     * @param maxSize
     * @return qint32
     */
    virtual qint32 write(const QByteArray* buf, quint32 maxSize) = 0;
    /**
     * @brief
     *
     * @param buf
     * @param maxSize
     * @return qint32
     */
    virtual qint32 read(QByteArray* buf, quint32 maxSize) = 0;

    /**
     * @brief
     *
     * @param buf
     * @return qint32
     */
    qint32 write(const QByteArray& buf);
    /**
     * @brief
     *
     * @param buf
     * @return qint32
     */
    qint32 read(QByteArray* buf);
    /**
     * @brief
     *
     * @param c
     * @return bool
     */
    bool write(char c);
    /**
     * @brief
     *
     * @param c
     * @return bool
     */
    bool read(char* c);

    /**
     * @brief
     *
     * @param timeout
     */
    void setTimeout(quint16 timeout);
    /**
     * @brief
     *
     */
    void showSettings(void);

protected slots:
    /**
     * @brief
     *
     */
    void setDefaults(void);

protected:
    QtUsb::DeviceFilter mFilter; /**< TODO: describe */
    QtUsb::DeviceConfig mConfig; /**< TODO: describe */
    quint16 mTimeout; /**< TODO: describe */
    bool mDebug; /**< TODO: describe */
    bool mConnected; /**< TODO: describe */
    QtUsb::DeviceSpeed mSpd; /**< TODO: describe */
};

#endif // QBASEUSB_H
