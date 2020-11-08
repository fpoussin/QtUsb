#include <QtTest/QtTest>
#include <QtUsb/QUsb>

class tst_QUsb : public QObject
{
    Q_OBJECT
private slots:
    void constructors();
    void assignment();
    void features();
    void staticFunctions();

private:
};

void tst_QUsb::constructors()
{
    QUsb usb;

    QCOMPARE(usb.logLevel(), QUsb::logInfo);
}

void tst_QUsb::assignment()
{
    QUsb usb;

    usb.setLogLevel(QUsb::logDebug);
    QCOMPARE(usb.logLevel(), QUsb::logDebug);

    usb.setLogLevel(QUsb::logNone);
    QCOMPARE(usb.logLevel(), QUsb::logNone);
}

void tst_QUsb::features()
{
    QUsb usb;
    usb.devices();
}

void tst_QUsb::staticFunctions()
{
    QUsb::devices();
}

QTEST_MAIN(tst_QUsb)
#include "tst_qusb.moc"
