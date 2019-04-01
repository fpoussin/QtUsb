#include <QtTest/QtTest>
#include <QtUsb/QUsbTransfer>

class tst_QUsbTransfer : public QObject
{
    Q_OBJECT
public:
    explicit tst_QUsbTransfer();

private slots:
    void constructors();
    void assignment();
    void polling();

private:
};

tst_QUsbTransfer::tst_QUsbTransfer()
{
}

void tst_QUsbTransfer::constructors()
{
    QUsbDevice dev;
    QUsbDevice::Endpoint ep_in = 81, ep_out = 1;
    QUsbTransfer handler1(&dev, QUsbTransfer::bulkTransfer, ep_in, ep_out);

    QCOMPARE(handler1.parent(), static_cast<QObject *>(&dev));
    QCOMPARE(handler1.status(), QUsbTransfer::transferCanceled);
    QVERIFY(handler1.type() == QUsbTransfer::bulkTransfer);
    QVERIFY(!handler1.polling());
    QCOMPARE(handler1.endpointIn(), ep_in);
    QCOMPARE(handler1.endpointOut(), ep_out);

    QUsbTransfer handler2(&dev, QUsbTransfer::interruptTransfer, ep_in, ep_out);
    QCOMPARE(handler2.parent(), static_cast<QObject *>(&dev));
    QCOMPARE(handler2.status(), QUsbTransfer::transferCanceled);
    QVERIFY(handler2.type() == QUsbTransfer::interruptTransfer);
    QVERIFY(!handler2.polling());
}

void tst_QUsbTransfer::assignment()
{
    QUsbDevice dev;
    QUsbDevice::Endpoint ep_in = 81, ep_out = 1;
    QUsbTransfer handler1(&dev, QUsbTransfer::bulkTransfer, ep_in, ep_out);

    QCOMPARE(handler1.parent(), static_cast<QObject *>(&dev));
    QVERIFY(handler1.type() == QUsbTransfer::bulkTransfer);

    QUsbTransfer handler2(&dev, QUsbTransfer::interruptTransfer, ep_in, ep_out);
    QCOMPARE(handler2.parent(), static_cast<QObject *>(&dev));
    QVERIFY(handler2.type() == QUsbTransfer::interruptTransfer);
}

void tst_QUsbTransfer::polling()
{
    QUsbDevice dev;
    QUsbDevice::Endpoint ep_in = 81, ep_out = 1;
    QUsbTransfer handler1(&dev, QUsbTransfer::bulkTransfer, ep_in, ep_out);

    QCOMPARE(handler1.parent(), static_cast<QObject *>(&dev));
    QVERIFY(!handler1.polling());
    QVERIFY(!handler1.poll());
    QVERIFY(handler1.open(QIODevice::ReadOnly));
    QVERIFY(handler1.isOpen());
    QVERIFY(handler1.poll());
    handler1.setPolling(true);
    QVERIFY(!handler1.poll());
    handler1.close();
    QVERIFY(!handler1.isOpen());

    QUsbTransfer handler2(&dev, QUsbTransfer::interruptTransfer, ep_in, ep_out);
    QCOMPARE(handler2.parent(), static_cast<QObject *>(&dev));
    QVERIFY(!handler2.polling());
    QVERIFY(handler2.open(QIODevice::ReadOnly));
    QVERIFY(handler2.isOpen());
    QVERIFY(handler2.polling());
    QVERIFY(!handler2.poll());
    handler2.setPolling(false);
    QVERIFY(handler2.poll());
    handler2.close();
    QVERIFY(!handler2.isOpen());
}

QTEST_MAIN(tst_QUsbTransfer)
#include "tst_qusbtransfer.moc"
