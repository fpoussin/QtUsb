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
    QUsb info;

    QCOMPARE(info.logLevel(), QUsb::logInfo);
}

void tst_QUsb::assignment()
{
    QUsb info;

    info.setLogLevel(QUsb::logDebug);
    QCOMPARE(info.logLevel(), QUsb::logDebug);

    info.setLogLevel(QUsb::logNone);
    QCOMPARE(info.logLevel(), QUsb::logNone);
}

void tst_QUsb::features()
{
    QUsb info;
    info.devices();
}

void tst_QUsb::staticFunctions()
{
    QUsb::devices();
}

QTEST_MAIN(tst_QUsb)
#include "tst_qusb.moc"
