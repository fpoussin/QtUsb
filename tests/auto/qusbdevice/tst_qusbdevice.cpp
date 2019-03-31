#include <QtTest/QtTest>
#include <QtUsb/QUsbDevice>

class tst_QUsbDevice : public QObject
{
    Q_OBJECT
public:
    explicit tst_QUsbDevice();

private slots:
    void constructors();
    void assignment();
    void states();
    void staticfuncs();

private:
};

tst_QUsbDevice::tst_QUsbDevice()
{
}

void tst_QUsbDevice::constructors()
{
    QUsbDevice dev;
    int timeout = QUsbDevice::DefaultTimeout; // We can't use references with this var
    const QUsbDevice::Config c = { 1, 0, 0 };

    QVERIFY(!dev.isConnected());
    QCOMPARE(dev.logLevel(), QUsbDevice::logInfo);
    QCOMPARE(dev.speed(), QUsbDevice::unknownSpeed);
    QCOMPARE(dev.speedString(), QByteArray("Unknown speed"));
    QCOMPARE(dev.config(), c);
    QCOMPARE(static_cast<uint>(dev.timeout()), static_cast<uint>(timeout));
}

void tst_QUsbDevice::assignment()
{
    QUsbDevice dev;
    QUsbDevice::Config c = { 1, 1, 1 };
    QUsbDevice::Id f = { 1234, 4321 };
    const quint16 timeout = (rand() % 200) + 10;

    dev.setConfig(c);
    dev.setId(f);

    QCOMPARE(dev.config(), c);
    QCOMPARE(dev.id(), f);

    dev.setTimeout(timeout);
    QCOMPARE(dev.timeout(), timeout);
}

void tst_QUsbDevice::states()
{
    QUsbDevice dev;

    dev.setLogLevel(QUsbDevice::logDebug);
    QCOMPARE(dev.logLevel(), QUsbDevice::logDebug);

    dev.setLogLevel(QUsbDevice::logNone);
    QCOMPARE(dev.logLevel(), QUsbDevice::logNone);
}

void tst_QUsbDevice::staticfuncs()
{
    QUsbDevice::devices();
}

QTEST_MAIN(tst_QUsbDevice)
#include "tst_qusbdevice.moc"
