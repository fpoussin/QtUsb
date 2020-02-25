#include <QtTest/QtTest>
#include <QtUsb/QHidDevice>

class tst_QHidDevice : public QObject
{
    Q_OBJECT
private slots:
    void constructors();

private:
};

void tst_QHidDevice::constructors()
{
    QHidDevice hid;

    QVERIFY(!hid.isOpen());
}

QTEST_MAIN(tst_QHidDevice)
#include "tst_qhiddevice.moc"
