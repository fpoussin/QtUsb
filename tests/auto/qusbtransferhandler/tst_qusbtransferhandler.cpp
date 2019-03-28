#include <QtTest/QtTest>
#include <QtUsb/QUsbTransferHandler>

class tst_QUsbTransferHandler : public QObject
{
    Q_OBJECT
public:
    explicit tst_QUsbTransferHandler();

private slots:
    void constructors();
    void assignment();
    void polling();

private:

};


tst_QUsbTransferHandler::tst_QUsbTransferHandler()
{

}

void tst_QUsbTransferHandler::constructors()
{
  QUsbDevice dev;
  QUsbDevice::Endpoint ep_in = 81, ep_out = 1;
  QUsbTransferHandler handler1(&dev, QUsbTransferHandler::bulkTransfer, ep_in, ep_out);

  QCOMPARE(handler1.parent(), static_cast<QObject*>(&dev));
  QCOMPARE(handler1.status(), QUsbTransferHandler::transferCanceled);
  QVERIFY(handler1.type() == QUsbTransferHandler::bulkTransfer);
  QVERIFY(!handler1.polling());
  QCOMPARE(handler1.endpointIn(), ep_in);
  QCOMPARE(handler1.endpointOut(), ep_out);

  QUsbTransferHandler handler2(&dev, QUsbTransferHandler::interruptTransfer, ep_in, ep_out);
  QCOMPARE(handler2.parent(), static_cast<QObject*>(&dev));
  QCOMPARE(handler2.status(), QUsbTransferHandler::transferCanceled);
  QVERIFY(handler2.type() == QUsbTransferHandler::interruptTransfer);
  QVERIFY(!handler2.polling());
}

void tst_QUsbTransferHandler::assignment()
{
  QUsbDevice dev;
  QUsbDevice::Endpoint ep_in = 81, ep_out = 1;
  QUsbTransferHandler handler1(&dev, QUsbTransferHandler::bulkTransfer, ep_in, ep_out);

  QCOMPARE(handler1.parent(), static_cast<QObject*>(&dev));
  QVERIFY(handler1.type() == QUsbTransferHandler::bulkTransfer);

  QUsbTransferHandler handler2(&dev, QUsbTransferHandler::interruptTransfer, ep_in, ep_out);
  QCOMPARE(handler2.parent(), static_cast<QObject*>(&dev));
  QVERIFY(handler2.type() == QUsbTransferHandler::interruptTransfer);

}

void tst_QUsbTransferHandler::polling()
{
  QUsbDevice dev;
  QUsbDevice::Endpoint ep_in = 81, ep_out = 1;
  QUsbTransferHandler handler1(&dev, QUsbTransferHandler::bulkTransfer, ep_in, ep_out);

  QCOMPARE(handler1.parent(), static_cast<QObject*>(&dev));
  QVERIFY(!handler1.polling());
  QVERIFY(!handler1.poll());
  QVERIFY(handler1.open(QIODevice::ReadOnly));
  QVERIFY(handler1.isOpen());
  QVERIFY(handler1.poll());
  handler1.setPolling(true);
  QVERIFY(!handler1.poll());
  handler1.close();
  QVERIFY(!handler1.isOpen());

  QUsbTransferHandler handler2(&dev, QUsbTransferHandler::interruptTransfer, ep_in, ep_out);
  QCOMPARE(handler2.parent(), static_cast<QObject*>(&dev));
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

QTEST_MAIN(tst_QUsbTransferHandler)
#include "tst_qusbtransferhandler.moc"
