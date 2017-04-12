#include "usbexample.h"
#include <QCoreApplication>

int main(int argc, char *argv[]) {
  QCoreApplication a(argc, argv);

  UsbExample example;

  return a.exec();
}
