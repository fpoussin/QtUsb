#include <QtTest/QtTest>
#include <QtUsb/QUsbEndpoint>

class tst_QUsbEndpoint : public QObject
{
    Q_OBJECT
private slots:
    void constructors();
    void polling();

private:
};

void tst_QUsbEndpoint::constructors()
{
    QUsbDevice dev;
    quint8 ep_in = 81;

    QUsbEndpoint handler_in(&dev, QUsbEndpoint::bulkEndpoint, ep_in);
    QVERIFY(handler_in.openMode() == QIODevice::NotOpen);
    QCOMPARE(handler_in.parent(), static_cast<QObject *>(&dev));
    QCOMPARE(handler_in.status(), QUsbEndpoint::transferCanceled);
    QVERIFY(handler_in.type() == QUsbEndpoint::bulkEndpoint);
    QVERIFY(!handler_in.polling());
    QCOMPARE(handler_in.endpoint(), ep_in);

    QUsbEndpoint handler_int(&dev, QUsbEndpoint::interruptEndpoint, ep_in);
    QVERIFY(handler_int.openMode() == QIODevice::NotOpen);
    QCOMPARE(handler_int.parent(), static_cast<QObject *>(&dev));
    QCOMPARE(handler_int.status(), QUsbEndpoint::transferCanceled);
    QVERIFY(handler_int.type() == QUsbEndpoint::interruptEndpoint);
    QVERIFY(!handler_int.polling());
}

void tst_QUsbEndpoint::polling()
{
    QUsbDevice dev;
    quint8 ep_in = 81;
    QUsbEndpoint handler1(&dev, QUsbEndpoint::bulkEndpoint, ep_in);

    QCOMPARE(handler1.parent(), static_cast<QObject *>(&dev));
    QVERIFY(!handler1.polling());
    QVERIFY(!handler1.poll());
    QVERIFY(handler1.open(QIODevice::ReadOnly));
    QVERIFY(handler1.openMode() == QIODevice::ReadOnly);
    QVERIFY(handler1.isOpen());
    QVERIFY(handler1.poll());
    handler1.setPolling(true);
    QVERIFY(!handler1.poll());
    handler1.close();
    QVERIFY(!handler1.isOpen());

    QUsbEndpoint handler2(&dev, QUsbEndpoint::interruptEndpoint, ep_in);
    QCOMPARE(handler2.parent(), static_cast<QObject *>(&dev));
    QVERIFY(!handler2.polling());
    QVERIFY(handler2.open(QIODevice::ReadOnly));
    QVERIFY(handler2.openMode() == QIODevice::ReadOnly);
    QVERIFY(handler2.isOpen());
    QVERIFY(handler2.polling());
    QVERIFY(!handler2.poll());
    handler2.setPolling(false);
    QVERIFY(handler2.poll());
    handler2.close();
    QVERIFY(!handler2.isOpen());
}

QTEST_MAIN(tst_QUsbEndpoint)
#include "tst_qusbendpoint.moc"
