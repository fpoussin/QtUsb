#include <QtTest/QtTest>
#include <QtUsb/QUsbDevice>

class tst_QUsbDevice : public QObject
{
    Q_OBJECT
public:
    explicit tst_QUsbDevice();

private slots:
    void initTestCase();
    void constructors();
    void assignment();

private:

};


tst_QUsbDevice::tst_QUsbDevice()
{

}

void tst_QUsbDevice::initTestCase()
{

}

void tst_QUsbDevice::constructors()
{

}

void tst_QUsbDevice::assignment()
{

}

QTEST_MAIN(tst_QUsbDevice)
#include "tst_qusbdevice.moc"
