#include <QtTest/QtTest>
#include <QtUsb/QUsbInfo>

class tst_QUsbInfo : public QObject
{
    Q_OBJECT
public:
    explicit tst_QUsbInfo();

private slots:
    void constructors();
    void assignment();
    void features();

private:

};

tst_QUsbInfo::tst_QUsbInfo()
{

}

void tst_QUsbInfo::constructors()
{
  QUsbInfo info;

  QVERIFY(!info.debug());

}

void tst_QUsbInfo::assignment()
{
  QUsbInfo info;

  info.setDebug(true);
  QVERIFY(info.debug());
  info.setDebug(false);
  QVERIFY(!info.debug());
}

void tst_QUsbInfo::features()
{
  QUsbInfo info;
  info.getPresentDevices();
}

QTEST_MAIN(tst_QUsbInfo)
#include "tst_qusbinfo.moc"
