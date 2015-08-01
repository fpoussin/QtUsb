#include <QCoreApplication>
#include <QTimer>
#include "usbexample.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QTimer timer;

    QObject::connect(&timer, SIGNAL(timeout()), &a, SLOT(quit()));

    timer.setInterval(1000);
    timer.setSingleShot(true);
    timer.start();

    UsbExample example;

    return a.exec();
}
