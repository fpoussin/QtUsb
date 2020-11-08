#include <QtTest/QtTest>
#include <QtUsb/QUsbDevice>

class tst_QUsbDevice : public QObject
{
    Q_OBJECT
private slots:
    void constructors();
    void assignment();
    void states();
    void staticfuncs();

private:
};

void tst_QUsbDevice::constructors()
{
    QUsbDevice dev;
    int timeout = QUsbDevice::DefaultTimeout; // We can't use references with this var
    const QUsb::Config c;

    QVERIFY(!dev.isConnected());
    QCOMPARE(dev.logLevel(), QUsb::logInfo);
    QCOMPARE(dev.speed(), QUsbDevice::unknownSpeed);
    QCOMPARE(dev.speedString(), QByteArray("Unknown speed"));
    QCOMPARE(dev.config(), c);
    QCOMPARE(static_cast<uint>(dev.timeout()), static_cast<uint>(timeout));
}

void tst_QUsbDevice::assignment()
{
    QUsbDevice dev;
    const QUsb::Config c(1, 2, 3);
    const QUsb::Id f(1234, 4321);
    const quint16 timeout = (rand() % 200) + 10;

    dev.setConfig(c);
    dev.setId(f);

    QCOMPARE(dev.config(), c);
    QCOMPARE(dev.id(), f);

    dev.setTimeout(timeout);
    QCOMPARE(dev.timeout(), timeout);

    const QUsb::Config c2 = c;
    const QUsb::Config c3(c);

    QCOMPARE(c, c2);
    QCOMPARE(c, c3);

    const QUsb::Id f2 = f;
    const QUsb::Id f3(f);

    QCOMPARE(f, f2);
    QCOMPARE(f, f3);
}

void tst_QUsbDevice::states()
{
    QUsbDevice dev;

    dev.setLogLevel(QUsb::logDebug);
    QCOMPARE(dev.logLevel(), QUsb::logDebug);

    dev.setLogLevel(QUsb::logNone);
    QCOMPARE(dev.logLevel(), QUsb::logNone);
}

void tst_QUsbDevice::staticfuncs()
{
}

QTEST_MAIN(tst_QUsbDevice)
#include "tst_qusbdevice.moc"
