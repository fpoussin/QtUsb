#ifndef QLIBUSB_H
#define QLIBUSB_H

#include <QString>
#include <QDebug>
#include <QByteArray>
#include <QtEndian>
#include <libusb-1.0/libusb.h>
#include "qbaseusb.h"
#include "qusb_compat.h"

/**
 * @brief
 *
 */
class QUSBSHARED_EXPORT QUsbDevice : public QBaseUsbDevice
{
    Q_OBJECT

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
     */
    ~QUsbDevice();

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
     * @param enable
     */
    void setDebug(bool enable);

private slots:    

private:
    /**
     * @brief
     *
     * @param error_code
     */
    void printUsbError(int error_code);
    libusb_device **mDevs; /**< TODO: describe */
    libusb_device_handle *mDevHandle; /**< TODO: describe */
    libusb_context *mCtx; /**< TODO: describe */
};
#endif // QLIBUSB_H
