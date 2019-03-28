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

private:

};


tst_QUsbDevice::tst_QUsbDevice()
{

}

void tst_QUsbDevice::constructors()
{
  QUsbDevice dev;
  int timeout = QUsbDevice::DefaultTimeout; // We can't use references with this var
  const QUsbDevice::Config c = {1,0,0};

  QVERIFY(!dev.isConnected());
  QVERIFY(!dev.debug());
  QCOMPARE(dev.speed(), QUsbDevice::unknownSpeed);
  QCOMPARE(dev.speedString(), QByteArray("Unknown speed"));
  QCOMPARE(dev.config(), c);
  QCOMPARE(static_cast<uint>(dev.timeout()), static_cast<uint>(timeout));
  }

void tst_QUsbDevice::assignment()
{
  QUsbDevice dev;
  QUsbDevice::Config c = {1, 1, 1};
  QUsbDevice::Filter f = {1234, 4321};
  int timeout = 10;

  dev.setConfig(c);
  dev.setFilter(f);

  QCOMPARE(dev.config(), c);
  QCOMPARE(dev.filter(), f);

  dev.setTimeout(timeout);
  QCOMPARE(static_cast<uint>(dev.timeout()), static_cast<uint>(timeout));
}

void tst_QUsbDevice::states()
{
  QUsbDevice dev;

  dev.setDebug(true);
  QVERIFY(dev.debug());

  dev.setDebug(false);
  QVERIFY(!dev.debug());

}

QTEST_MAIN(tst_QUsbDevice)
#include "tst_qusbdevice.moc"
