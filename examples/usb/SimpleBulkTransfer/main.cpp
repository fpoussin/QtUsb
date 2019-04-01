#include "usbexample.h"
#include <QCoreApplication>
#include <QTimer>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QTimer timer;

    qInfo("SimpleBulkTransfer");

    QObject::connect(&timer, SIGNAL(timeout()), &a, SLOT(quit()));

    timer.setInterval(3000);
    timer.setSingleShot(true);
    timer.start();

    UsbExample example;

    return a.exec();
}
